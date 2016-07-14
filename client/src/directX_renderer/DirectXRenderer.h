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

#ifndef _included_directXRenderer_h
#define _included_directXRenderer_h

#include "VideoRenderer.h"
#include "MUD/threading/ScopeLock.h"
#include "MUD/threading/SimpleLock.h"

#include <windows.h>
#include <d3d9.h>
#include <D3dx9core.h>
#include <sstream>
#include <string>
#include <Util.h>
#include <unordered_map>
#include "DirectXTextBox.h"
#include "DumpFrameTest.h"

/**
 * This is a directX renderer
 * It needs
 *      A window handle, provided with setWindowHandle to draw on
 *      An XStxRawVideoFrame, provided with postFrame, to use as texture for rendering
 */
class DirectXRenderer : public VideoRenderer
{
public:
    //constructor
    DirectXRenderer();
    //destructor
    ~DirectXRenderer();

    /**
     * Set up the size for the backbuffer
     * System-side and GPU-side textures are always 1080p
     * We change the resolution by changing the uv co-cords of the fullscreen quad to crop
     * the textures, instead of changing GPU texture sizes
     */
    virtual bool init() { return init(1280,720); }

    /**
     * Set up the size for the backbuffer
     * System-side and GPU-side textures are always 1080p
     * We change the resolution by changing the uv co-cords of the fullscreen quad to crop
     * the textures, instead of changing GPU texture sizes
     */
    bool init(uint32_t w, uint32_t h);

    /**
     * Renders the image onto the display. We do this by drawing a full-screen quad
     * and using a fragment shader to convert the YUV data into the viewable RGB image.
     */     
    virtual int draw();
 
    /**
     * This function:
     * Check for resolution change in mFrame and adapt(change quad uv cord) if neccesary
     * Upload mFrame to GPU texture, get ready for draw to be called
     */
    virtual void render();

    /**
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool receivedClientConfiguration(const XStxClientConfiguration* config);

    /**
     * Provide Chroma sampling capability
     */
    virtual bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);

    virtual void clearScreen();
    virtual void post();

    /**
     * provide the window handle as canvas for directX graphics. this needs to be 
     * called before setupDirectX is called.
     */
    void setWindowHandle(HWND window);
/*
    //should be on the same thread that calls draw()
    void setFpsText(char *fps);
    
    void setStatusText(const char *status);
    void setTcpReconnectText(const char* text);
*/
    uint32_t getLastSetWidth();
    uint32_t getLastSetHeight();

    //text box manipulation functions
    void createTextBox(std::string name, RECT position,bool visibility = true);
    void setTextBoxText(std::string textBoxName, const char* text);
    void showTextBox(std::string textBoxName);
    void hideTextBox(std::string textBoxName);
    void showTimedTextBox(std::string textBoxName, uint32_t timeOutMs);

private:

    bool setupDirectX(HWND window);
    void releaseResource();
    void setupTextBoxes();
    void compileShaders();

    /**
     * Upload mFrame to GPU texture, get ready for rendering
     */
    void copyToTexture(LPDIRECT3DDEVICE9 direct3DDevice,
                       LPDIRECT3DTEXTURE9 systemText,
                       LPDIRECT3DTEXTURE9 gpuText,
                       unsigned char * data, int height, int width, 
                       int padding, int reservedSize);
    /**
     * Setup D3d device, allocate textures if they are null and bind the textures
     * this method need to be called everytime direct3DDevice is reset. For example
     * it need to be called after the device switch from D3DERR_DEVICENOTRESET to D3D_OK
     * http://msdn.microsoft.com/en-us/library/windows/desktop/bb174714%28v=vs.85%29.aspx
     */
    void init3DDevice();
    //this need to be recalled everytime d3dDevice is lost and restored
    void initGPUTexturesForYUVFrame();
    //these CPU side textures persist so only need to be called once when 
    //we get the resolultion(YUV444 or 420) from the server
    void initCPUTexturesForYUVFrame();

    /**
     * Check if the current frame's resolution is different from the last rendered one and
     * change the UV coordinate of the full screen quad to update the new resolution.
     * TODO:The back buffer size, however, is fixed at 720p, this needs to change if we want to 
     * support 1080p in the future. 
     */
    void checkAndUpdateResolution();

    /**
     * Copy the Y, U and V plane texture to the corresponding GPU textures. The size of 
     * U and V planes are assumed to be half of the size of the Y plane.
     */
    void convertAndCopyFrameData();

    //void renderFps();
    //void renderErrorText();
    //void renderTcpReconnectStatusText();

    void handleStreamingStopRender();

    void DrawTextBox(DXTextBox *textBox);

    bool mDeviceLost;
    mud::SimpleLock mDeviceLostLock;
    bool isDeviceLost();

    HWND mWindowHandle;
    HDC mDeviceContext;

    LPDIRECT3DDEVICE9 direct3DDevice;
    LPDIRECT3DVERTEXBUFFER9 quadBuffer;
    
    LPDIRECT3DTEXTURE9 copyToTextureY;
    LPDIRECT3DTEXTURE9 renderTextureY;
    LPDIRECT3DTEXTURE9 copyToTextureU;
    LPDIRECT3DTEXTURE9 renderTextureU;
    LPDIRECT3DTEXTURE9 copyToTextureV;
    LPDIRECT3DTEXTURE9 renderTextureV;

    //default worldViewProj for rendering ful screen quad
    D3DXMATRIXA16 mMatWorldViewProj;
    LPD3DXCONSTANTTABLE mVertexShaderConstant;

    //yuv to rgb full screen pixel shader
    LPDIRECT3DVERTEXSHADER9 vertexShader;
    LPDIRECT3DVERTEXDECLARATION9  vertexDecl;
    LPDIRECT3DPIXELSHADER9 YUVtoRGBPixelShader;
    LPD3DXCONSTANTTABLE pixelShaderConstans;

    //solid color pixel shader 
    LPDIRECT3DPIXELSHADER9 solidColPixelShader;

    LPD3DXFONT font;

    D3DPRESENT_PARAMETERS d3PresentParams;
    bool mIsQuadBufferValid;
    bool mFirstYUVFrameReceived;
    //for on-the-fly resolution changes
    uint32_t mLastSetHeight;
    uint32_t mLastSetWidth;

    //storing the current fps text and status text
    std::unordered_map<std::string,DXTextBox*> mTextBoxes;

    //loop through all textboxes and display currently shown textboxes
    mud::SimpleLock mTextBoxesLock;
    void displayTextBoxes();

    XStxChromaSampling mChromaSampling;
    bool mUseYuv444;

    // for automated testing, object to dump frames
    DumpFrameTest* mDumpFrameTest;
};



#endif
