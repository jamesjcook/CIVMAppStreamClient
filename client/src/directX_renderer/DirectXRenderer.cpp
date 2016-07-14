/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#include "DirectXRenderer.h"
#include "MUD/threading/ThreadUtil.h"

static const char * VS_UNIFORM_WORLD_VIEW_PROJ = "WorldViewProj";

#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=NULL; } }

struct D3DVERTEX
    {
        D3DXVECTOR3 pos;
        D3DXVECTOR2 tex;
    };

//quad vertex shader
static const char * vertexShaderStr = ""
        "struct VS_INPUT { \n"
        "    float4 Position : POSITION; \n"
        "    float2 Texture : TEXCOORD0; \n"
        "}; \n"
        "struct VS_OUTPUT { \n"
        "    float4 Position   : POSITION; \n"
        "    float2 Texture    : TEXCOORD0; \n"
        "}; \n"
        "float4x4 WorldViewProj; \n"
        "VS_OUTPUT main(in VS_INPUT In) { \n"
        "    VS_OUTPUT Out; \n"
        "    Out.Position = mul(In.Position, WorldViewProj); \n"
        "    Out.Texture = In.Texture; \n"
        "    return Out; \n"
        "} \n";

//Note that, for directX 9, need to shift the texture by 0.5 texel size:
//http://msdn.microsoft.com/en-us/library/windows/desktop/bb219690%28v=vs.85%29.aspx
char * pixelShaderStr = ""
    "struct PS_INPUT {  \n"
    "    float4 Position : POSITION; \n"
    "    float2 Texture : TEXCOORD0; \n"
    "}; \n"

    "struct PS_OUTPUT { \n"
    "    float4 Color : COLOR0; \n"
    "}; \n"
    "sampler2D Tex0; \n"
    "sampler2D Tex1; \n"
    "sampler2D Tex2; \n"
    "float2 resolution; \n"
    "PS_OUTPUT main(in PS_INPUT In) { \n"
    "    PS_OUTPUT  Out; \n"
    "    float r,g,b,y,u,v; \n"
    "    Out.Color = float4(0.0, 0.0, 0.0, 1.0); \n"
    "    y=tex2D(Tex0, In.Texture + float2(0.5/resolution.x,0.5/resolution.y)).r; \n"
    "    u=tex2D(Tex1, In.Texture + float2(0.5/resolution.x,0.5/resolution.y)).r; \n"
    "    v=tex2D(Tex2, In.Texture + float2(0.5/resolution.x,0.5/resolution.y)).r; \n"
    "    y=1.1643*(y-0.0625); \n"
    "    u=u-0.5; \n"
    "    v=v-0.5; \n"
    "    r=y+1.5958*v; \n"
    "    g=y-0.39173*u-0.81290*v; \n"
    "    b=y+2.017*u; \n"
    "    Out.Color.r = r; \n"
    "    Out.Color.g = g; \n"
    "    Out.Color.b = b; \n"
    "    return Out; \n"
    "} \n";


//solid color pixel shader, used for drawing text background
char * solidColorPixelShaderStr = ""
    "struct PS_INPUT {  \n"
    "    float4 Position : POSITION; \n"
    "    float2 Texture : TEXCOORD0; \n"
    "}; \n"
    "struct PS_OUTPUT { \n"
    "    float4 Color : COLOR0; \n"
    "}; \n"
    "PS_OUTPUT main1(in PS_INPUT In) { \n"
    "    PS_OUTPUT  Out; \n"
    "    Out.Color = float4(0.2,0.2,0.2, 0.8); \n"
    "    return Out; \n"
    "} \n";


/**
 * DirectXRenderer
 */

    /** Constructor */
    DirectXRenderer::DirectXRenderer() :
        copyToTextureY(NULL),
        renderTextureY(NULL),
        copyToTextureU(NULL),
        renderTextureU(NULL),
        copyToTextureV(NULL),
        renderTextureV(NULL),
        mWindowHandle(NULL),
        mDeviceContext(NULL),
        mIsQuadBufferValid(false),
        mLastSetHeight(0),
        mLastSetWidth(0),
        mDeviceLost(false),
        mDumpFrameTest(NULL),
        mChromaSampling(XSTX_CHROMA_SAMPLING_UNKNOWN),
        mUseYuv444(false),
        mFirstYUVFrameReceived(false)
    {
    }

    /** Destructor */
    DirectXRenderer::~DirectXRenderer()
    {
        ReleaseDC(mWindowHandle, mDeviceContext);
        //delete all textboxes
        typedef std::unordered_map<std::string, DXTextBox*> Mymap; 
        for (Mymap::iterator it = mTextBoxes.begin(); it != mTextBoxes.end(); ++it)
        {
            delete it->second;
            it->second = NULL;
        }
        
        if (mDumpFrameTest)
        {
            delete mDumpFrameTest;
        }
    }

    //Overload base class method. After streaming stops, we render the last
    //received frame and the error text. 
    void DirectXRenderer::handleStreamingStopRender()
    {
        /*
        renderClear();
        renderImage();
        renderErrorText(mErrorText.c_str(), mErrorText.size());
        post();
        mud::ThreadUtil::sleep(30);
        */
    }
	bool DirectXRenderer::init(uint32_t w, uint32_t h)
    {
        mWidth = w;
        mHeight = h;

        // for automated testing, check for environment var and initialize test object
        char* frameDumpTimes = getenv("XSTX_FRAME_TEST");
        if (frameDumpTimes != NULL)
        {
            mDumpFrameTest = new DumpFrameTest;
            mDumpFrameTest->parseEnvironmentVariable(frameDumpTimes);
        }

        if(mWindowHandle == NULL)
        {
            printf("DirectX init failed, windowHandle has not been set.");
            return false;
        }
        else
            return setupDirectX(mWindowHandle);
    }

    void DirectXRenderer::setWindowHandle(HWND windowHandle)
    {
        mWindowHandle = windowHandle;        
    }
    void DirectXRenderer::DrawTextBox(DXTextBox *textBox)
    {
        mVertexShaderConstant->SetMatrix(
            direct3DDevice,
            VS_UNIFORM_WORLD_VIEW_PROJ,
            &textBox->getOffsetMatrix()
            );
        direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
        
        font->DrawText(
            NULL, 
            textBox->getText(), 
            -1, 
            &(textBox->getTextPosition()),
            textBox->getFormat(),
            textBox->getColor());
    }

    void DirectXRenderer::displayTextBoxes()
    {
        mud::ScopeLock sl(mTextBoxesLock);
        typedef std::unordered_map<std::string, DXTextBox*> Mymap; 
        for (Mymap::iterator it = mTextBoxes.begin(); it != mTextBoxes.end(); ++it)
        {
                it->second->update();
                if(it->second->isVisible())
                    DrawTextBox(it->second);
        }
    }
    bool DirectXRenderer::setupDirectX(HWND window)
    {
        mDeviceContext = GetDC(window);

        LPDIRECT3D9 direct3D = Direct3DCreate9(D3D_SDK_VERSION);
        if (direct3D == NULL) {
            return false;
        }
        //size
        ZeroMemory(&d3PresentParams,sizeof(d3PresentParams));
        //fullscreen
        d3PresentParams.Windowed = true;
        d3PresentParams.hDeviceWindow = window;
        d3PresentParams.BackBufferWidth = mWidth;
        d3PresentParams.BackBufferHeight = mHeight;
        d3PresentParams.BackBufferFormat = D3DFMT_A8R8G8B8; // directx has yuv formats!
        d3PresentParams.BackBufferCount = 1;
        d3PresentParams.MultiSampleType = D3DMULTISAMPLE_NONE;
        d3PresentParams.MultiSampleQuality = 0;
        d3PresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;

        if (FAILED(
            direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL,
                window, D3DCREATE_SOFTWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
                &d3PresentParams, &direct3DDevice)
            )) {
            return false;
        }

        compileShaders();
        init3DDevice();
        setupTextBoxes();
        return true;
    }
  
    void DirectXRenderer::releaseResource()
    {
        font->OnLostDevice();
        SAFE_RELEASE(quadBuffer)
        SAFE_RELEASE(renderTextureU)
        SAFE_RELEASE(renderTextureY)
        SAFE_RELEASE(renderTextureV)
    }

    void DirectXRenderer::createTextBox(std::string name, RECT position,bool visibility)
    {
        mud::ScopeLock sl(mTextBoxesLock);
        //check for existing key
        if(mTextBoxes.count(name) != 0)
        {
            mTextBoxes[name]->setPosition(position, font);
            return;
        }
        DXTextBox* newBox = new DXTextBox(mWidth, mHeight, DT_CENTER,0xffffffff,visibility);
        newBox->setPosition(position, font);
        mTextBoxes[name] = newBox;
    }
    void DirectXRenderer::setupTextBoxes()
    {
        RECT frameRateFontPos;
        frameRateFontPos.top = 5;
        frameRateFontPos.left = mWidth / 2 - 40;
        frameRateFontPos.right = mWidth / 2 + 40;
        frameRateFontPos.bottom = 35;
        createTextBox("framerate", frameRateFontPos);
      
        RECT resolutionTextPos;
        resolutionTextPos.top = 5;
        resolutionTextPos.left = mWidth - 125;
        resolutionTextPos.right = mWidth - 5;
        resolutionTextPos.bottom = 35;
        createTextBox("resolution", resolutionTextPos);
     
        RECT statusTextPos;
        statusTextPos.top = ( mHeight / 2 ) - 15;
        statusTextPos.left = ( mWidth / 10 );
        statusTextPos.right = mWidth - statusTextPos.left;
        statusTextPos.bottom = ( mHeight / 2 ) + 15;
        createTextBox("status", statusTextPos, false);
        
        //set the reconnect box on top of the status text box
        RECT reconnectStatusTextPos;
        reconnectStatusTextPos.top = statusTextPos.top + 30;
        reconnectStatusTextPos.left = statusTextPos.left;
        reconnectStatusTextPos.right = statusTextPos.right;
        reconnectStatusTextPos.bottom = statusTextPos.bottom + 30;
        createTextBox("reconnect", reconnectStatusTextPos, false);
        
        //mToaster = new DXToasterController(mWidth, mHeight, 16, 10);
    }
    void DirectXRenderer::setTextBoxText(std::string textBoxName, const char* text)
    {
        if(mTextBoxes.count(textBoxName) != 0)
        {
            mud::ScopeLock sl(mTextBoxesLock);
            mTextBoxes[textBoxName]->setText(text);
        }

    }

    void DirectXRenderer::showTextBox(std::string textboxName)
    {
        if(mTextBoxes.count(textboxName) != 0)
        {
            mud::ScopeLock sl(mTextBoxesLock);
            mTextBoxes[textboxName]->show();
        }
    }

    void DirectXRenderer::hideTextBox(std::string textboxName)
    {
        if(mTextBoxes.count(textboxName) != 0)
        {
            mud::ScopeLock sl(mTextBoxesLock);
            mTextBoxes[textboxName]->hide();
        }
    }

    void DirectXRenderer::showTimedTextBox(std::string textBoxName, uint32_t timeOutMs)
    {
        if(mTextBoxes.count(textBoxName) != 0)
        {
            mud::ScopeLock sl(mTextBoxesLock);
            mTextBoxes[textBoxName]->startCountDown(timeOutMs);
        }
    }
    void DirectXRenderer::compileShaders() {

        LPD3DXBUFFER code;
        LPD3DXBUFFER errors;
        //compile vertex shader
        if (FAILED(D3DXCompileShader(vertexShaderStr, (UINT) strlen(vertexShaderStr),
            NULL, NULL, "main", "vs_1_1", 0, &code, &errors, &mVertexShaderConstant))) {
                printf("Failed to create vertex shader: %s\n", errors->GetBufferPointer());
                return;
        }
        direct3DDevice->CreateVertexShader((DWORD*)code->GetBufferPointer(), &vertexShader);
        code->Release();


        //compile full screen quad pixel shader
        if (FAILED(D3DXCompileShader(pixelShaderStr, (UINT)strlen(pixelShaderStr),
            NULL, NULL, "main", "ps_2_0", 0, &code, &errors,  &pixelShaderConstans))) {
                printf("Failed to create pixel shader: %s\n", errors->GetBufferPointer());
                return;
        }
        direct3DDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &YUVtoRGBPixelShader);
        code->Release();


        //compile solid color pixel shader
        if (FAILED(D3DXCompileShader(solidColorPixelShaderStr, (UINT)strlen(solidColorPixelShaderStr),
            NULL, NULL, "main1", "ps_2_0", 0, &code, &errors,  NULL))) {
                printf("Failed to create pixel shader: %s\n", errors->GetBufferPointer());
                return;
        }
        direct3DDevice->CreatePixelShader((DWORD*)code->GetBufferPointer(), &solidColPixelShader);
        code->Release();

        //set up Vertex Shader declaration
        D3DVERTEXELEMENT9 decl[] = {{0,
                                     0,
                                     D3DDECLTYPE_FLOAT3,
                                     D3DDECLMETHOD_DEFAULT,
                                     D3DDECLUSAGE_POSITION,
                                     0},
                                    {0,
                                     12,
                                     D3DDECLTYPE_FLOAT2,
                                     D3DDECLMETHOD_DEFAULT,
                                     D3DDECLUSAGE_TEXCOORD,
                                     0},
                                    D3DDECL_END()};
        
        direct3DDevice->CreateVertexDeclaration(decl, &vertexDecl);

        D3DXMATRIXA16 matWorld, matView, matProj;
        direct3DDevice->GetTransform(D3DTS_WORLD, &matWorld);
        direct3DDevice->GetTransform(D3DTS_VIEW, &matView);
        direct3DDevice->GetTransform(D3DTS_PROJECTION, &matProj);

        mMatWorldViewProj = matWorld * matView * matProj;
    }

    void DirectXRenderer::copyToTexture(LPDIRECT3DDEVICE9 direct3DDevice,
                       LPDIRECT3DTEXTURE9 systemText,
                       LPDIRECT3DTEXTURE9 gpuText,
                       unsigned char * data, int height, int width, int padding, int reservedSize) {
        D3DLOCKED_RECT lockedRect;
        if (FAILED(systemText->LockRect(0, &lockedRect, NULL, NULL)))
            printf("\nDirectXRenderWindow error:LockRect failed");
        unsigned char * ptr = data;
        unsigned char* bits = ( unsigned char* )lockedRect.pBits; 
     
        for (int i=0;i<height;i++)
        { 
            memcpy(
                bits + i*reservedSize,
                ptr,
                width
                ); 
            ptr+= width + padding;

        }
        systemText->UnlockRect(0);
        direct3DDevice->UpdateTexture(systemText, gpuText);
    }

    void DirectXRenderer::initCPUTexturesForYUVFrame()
    {
        int const pWidth = mUseYuv444 ? 1920 : 1920 / 2;
        int const pHeight= mUseYuv444 ? 1080 : 1080 / 2;
        //create system-side textures
        if (copyToTextureY == NULL) 
        {
            direct3DDevice->CreateTexture(1920, 1080,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &copyToTextureY, NULL);
        }
        if (copyToTextureU == NULL) 
        {
            direct3DDevice->CreateTexture(pWidth, pHeight,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &copyToTextureU, NULL);

        }
        if (copyToTextureV == NULL) 
        {
            direct3DDevice->CreateTexture(pWidth, pHeight,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_SYSTEMMEM, &copyToTextureV, NULL);

        }
    }
    void DirectXRenderer::initGPUTexturesForYUVFrame()
    {
        int const pWidth = mUseYuv444 ? 1920 : 1920 / 2;
        int const pHeight= mUseYuv444 ? 1080 : 1080 / 2;
        //create GPU-side textures
        if(renderTextureY == NULL)
        {
            direct3DDevice->CreateTexture(1920, 1080,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT , &renderTextureY, NULL);
        }

        if(renderTextureU == NULL)
        {
            direct3DDevice->CreateTexture(pWidth, pHeight,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT , &renderTextureU, NULL);
        }
       
        if(renderTextureV == NULL)
        {
            direct3DDevice->CreateTexture(pWidth, pHeight,
                0, D3DUSAGE_DYNAMIC, D3DFMT_L8, D3DPOOL_DEFAULT , &renderTextureV, NULL);
        }

        direct3DDevice->SetTexture(0, renderTextureY);
        direct3DDevice->SetTexture(1, renderTextureU);
        direct3DDevice->SetTexture(2, renderTextureV);
    }

    void DirectXRenderer::init3DDevice()
    { 
        //init render state
        direct3DDevice->SetRenderState(D3DRS_AMBIENT, RGB(255, 255, 255));
        direct3DDevice->SetRenderState(D3DRS_LIGHTING, false);
        direct3DDevice->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);
        direct3DDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        direct3DDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        direct3DDevice->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
        direct3DDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        direct3DDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
        direct3DDevice->SetVertexDeclaration(vertexDecl);
        direct3DDevice->SetVertexShader(vertexShader);
        //send the window resolution to the pixel shader for half texel size calculation:
        float res[2] = { (float) mWidth, (float) mHeight};
        pixelShaderConstans->SetFloatArray(direct3DDevice,"resolution", res, 2);

        D3DXFONT_DESC fontDesc = {
            16,
            0,
            400,
            0,
            false,
            DEFAULT_CHARSET, 
            OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS,
            DEFAULT_PITCH,
            "Arial"};

        D3DXCreateFontIndirect(direct3DDevice, &fontDesc, &font);

        // make our rendering plane. Ensure we don't render past the width
        D3DVERTEX quadVert[] = {
            { D3DXVECTOR3(-1.0f , -1.0f , 0.0f),  D3DXVECTOR2(0, 1)},
            { D3DXVECTOR3(1.0f , -1.0f , 0.0f),   D3DXVECTOR2(1, 1)},
            { D3DXVECTOR3(-1.0f , 1.0f , 0.0f),   D3DXVECTOR2(0, 0)},
            { D3DXVECTOR3(1.0f , 1.0f , 0.0f),    D3DXVECTOR2(1, 0)}
        };
        direct3DDevice->CreateVertexBuffer(sizeof(quadVert),
                                           D3DUSAGE_WRITEONLY,
                                           0,
                                           D3DPOOL_DEFAULT,
                                           &quadBuffer,
                                           NULL);
        void * buffer;
        quadBuffer->Lock(0, 0, (void**)&buffer, 0);            
        memcpy(buffer, quadVert, sizeof(quadVert));
        quadBuffer->Unlock();
        mLastSetWidth = 0;
        mLastSetHeight = 0;
        mIsQuadBufferValid = true;

        direct3DDevice->SetStreamSource(0, quadBuffer, 0, sizeof(D3DVERTEX));
    }

    void DirectXRenderer::checkAndUpdateResolution()
    {
        //change the uv coord if resolution changed.
        if(mLastSetWidth != mFrame->mWidth || mLastSetHeight != mFrame->mHeight)
        {
            D3DVERTEX quadVert[] = {
                { D3DXVECTOR3(-1.0f , -1.0f , 0.0f),  D3DXVECTOR2(0.0f, (float)(mFrame->mHeight)/1080.0f)},
                { D3DXVECTOR3(1.0f , -1.0f , 0.0f),   D3DXVECTOR2((float)(mFrame->mWidth)/1920.0f, (float)(mFrame->mHeight)/1080.0f)},
                { D3DXVECTOR3(-1.0f , 1.0f , 0.0f),   D3DXVECTOR2(0.0f, 0.0f)},
                { D3DXVECTOR3(1.0f , 1.0f , 0.0f),    D3DXVECTOR2((float)(mFrame->mWidth)/1920.0f, 0.0f)}
            };
            void * buffer;
            if( ! FAILED(  quadBuffer->Lock(0, 0, (void**)&buffer, NULL)))
            {
               
                memcpy(buffer, quadVert, sizeof(quadVert));
                quadBuffer->Unlock();
                std::stringstream ss;
                ss << mFrame->mWidth <<"x"<<mFrame->mHeight;
                if(mTextBoxes.count("resolution") != 0)
                    mTextBoxes["resolution"]->setText(ss.str().c_str());
            }
            mLastSetWidth = mFrame->mWidth;
            mLastSetHeight = mFrame->mHeight; 
        }
     
    }
    void DirectXRenderer::convertAndCopyFrameData()
    {
        checkAndUpdateResolution();
        // copy the data from the raw YUV frame to the GPU textures
        copyToTexture(
            direct3DDevice,
            copyToTextureY,
            renderTextureY,
            (unsigned char*)mFrame->mPlanes[0],
            mFrame->mHeight, mFrame->mWidth,  mFrame->mStrides[0] - mFrame->mWidth
            ,1920);
        if (mUseYuv444)
        {
            copyToTexture(
                direct3DDevice,
                copyToTextureU,
                renderTextureU,
                (unsigned char*)mFrame->mPlanes[1],
                mFrame->mHeight, mFrame->mWidth, mFrame->mStrides[1] - mFrame->mWidth
                ,1920);
            copyToTexture(
                direct3DDevice,
                copyToTextureV,
                renderTextureV,
                (unsigned char*)mFrame->mPlanes[2], 
                mFrame->mHeight, mFrame->mWidth, mFrame->mStrides[2] - mFrame->mWidth
                ,1920);
        }
        else
        {
            copyToTexture(
                direct3DDevice,
                copyToTextureU,
                renderTextureU,
                (unsigned char*)mFrame->mPlanes[1],
                (mFrame->mHeight / 2) , ( mFrame->mWidth / 2),  mFrame->mStrides[1] - mFrame->mWidth / 2
                ,1920/2);
            copyToTexture(
                direct3DDevice,
                copyToTextureV,
                renderTextureV,
                (unsigned char*)mFrame->mPlanes[2], 
                (mFrame->mHeight / 2) , ( mFrame->mWidth / 2),  mFrame->mStrides[2] - mFrame->mWidth / 2
                ,1920/2);
        }

        // for automated testing
        if (mDumpFrameTest)
        {
            mDumpFrameTest->setTimeStamps(mFrame);
        }
    }

    void DirectXRenderer::clearScreen()
    {    
    }

    void DirectXRenderer::render()
    {
        convertAndCopyFrameData();
    }

    bool DirectXRenderer::isDeviceLost()
    {
        mud::ScopeLock sl(mDeviceLostLock);
        return mDeviceLost;            
    }

    int DirectXRenderer::draw()
    {
		int nFrameRendered = 0;

        if (mFrame == NULL)
        {
           // displayTextBoxes();
            mud::ThreadUtil::sleep(5);
            //return 0;
		}

		//check for the status of the d3d device, only render if it is
        //in OK state. When some events happen, such as the screen resolution
        //changes, or the screen is locked, the device state will change from
        //OK to lost state, in that case no render should be carried out. 
        HRESULT r =  direct3DDevice->TestCooperativeLevel();
        if (r != 0)
        {  
            if (r == D3DERR_DEVICELOST)
            {
                //set deviceLost to true so dont need to convert and upload mFrame
                //during the time the device is lost
                {
                    mud::ScopeLock sl(mDeviceLostLock);
                    mDeviceLost = true;                   
                }
                return 0;
            }
            else  if (r == D3DERR_DEVICENOTRESET)
            {
                //device just came out of lost state and need to be reset
                //to comeback to operational state:
                releaseResource();
                //reset the device here
                HRESULT r1 = direct3DDevice->Reset(&d3PresentParams);

                if (r1 != D3D_OK)
                {
                    printf("\nd3dDevice reset failed\n");
                    return 0;
                }
                {
                    mud::ScopeLock sl(mDeviceLostLock);
                    mDeviceLost = false;                   
                }
                printf("\nD3dDevice reset.");
                init3DDevice();
                initGPUTexturesForYUVFrame();
                return 0;
            }
        }
        nFrameRendered = checkQueue();
        if(mFirstYUVFrameReceived == false && nFrameRendered > 0)
        {
            mFirstYUVFrameReceived = true;
        }

        direct3DDevice->Clear(
            0,
            NULL,
            D3DCLEAR_TARGET,
            D3DCOLOR_XRGB(0, 0, 0),
            1.0f,
            0
            );
        //draw a full-screen quad
        //if (mIsQuadBufferValid && mLastSetHeight != 0)
        if (mIsQuadBufferValid)
        {
            direct3DDevice->BeginScene();
            //only render the frame if we have received at least 1
            if(mFirstYUVFrameReceived)
            {
                direct3DDevice->SetPixelShader(YUVtoRGBPixelShader);
                mVertexShaderConstant->SetMatrix(
                    direct3DDevice,
                    VS_UNIFORM_WORLD_VIEW_PROJ,
                    &mMatWorldViewProj
                    );
                direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);
            }


            //TODO: This is a hack to minimize the number of shader switches 
            //this only work with the assumption that we have only 2 shaders
            direct3DDevice->SetPixelShader(solidColPixelShader);
            // for automated testing, FPS counter text throws off frame data, enable if we aren't dumping frames
            if (mDumpFrameTest == NULL)
            {
                //draw textboxes on top of the frame texture
                displayTextBoxes();
            }
            direct3DDevice->EndScene();
        }
        return nFrameRendered;
    }

    void DirectXRenderer::post()
    {
        // for automated testing
        if (mDumpFrameTest)
        {
            mDumpFrameTest->dumpFrame(mHeight, mWidth, mWindowHandle);
        }
        direct3DDevice->Present(NULL, NULL, NULL, NULL);
    }
    
    uint32_t DirectXRenderer::getLastSetWidth()
    {
        return mLastSetWidth;
    }
    uint32_t DirectXRenderer::getLastSetHeight()
    {
        return mLastSetHeight;
    }

bool DirectXRenderer::receivedClientConfiguration(const XStxClientConfiguration* config)
{
    // YUV420 or YUV444
    mChromaSampling = config->mChromaSampling;
    mUseYuv444 = mChromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
    //reserve texture memory and vertex buffer
    /*
     * Look like GPU resource allocation needs not to be on the same thread
     * that call d3dReset device( but d3dDevice creation does need)
     */
    initCPUTexturesForYUVFrame();
    initGPUTexturesForYUVFrame();

    return isChromaSamplingSupported(mChromaSampling);
}

bool DirectXRenderer::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420
        || chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
}
