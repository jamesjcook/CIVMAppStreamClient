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

#include <memory>
#include <d3d11.h>
#include <stdio.h>
#include <DirectXMath.h>
#include <DXGI.h>

#include "Audio/Audio.h"
#include "Game.h"

#include "Capture/IScreenCapture.h"
// Capture methods that we can use
#include "Capture/GDICapture.h"
#include "Capture/DXCapture.h"

#pragma warning (push)

#if _MSC_VER && !__INTEL_COMPILER
    // Disable warnings about macro re-definitions if we're using the DX SDK
    // http://stackoverflow.com/questions/19336841/i-get-a-lot-of-macro-redefinition-when-using-directx-10
    #pragma warning (disable: 4005)
#endif

#pragma warning (pop)

using namespace DirectX;

const unsigned int MAX_MESH_COUNT = 20;

WAVEFORMATEXTENSIBLE XSTX_DX1_G_WAVE_DATA = {0};
XAUDIO2_BUFFER XSTX_DX1_G_WAVE_BUFFER = {0};
XMVECTOR XSTX_DX1_G_EYE = {0.0f, 5.0f, -5.0f};
XMVECTOR XSTX_DX1_G_LOOK_AT = {0.0f, 0.0f, 0.0f};
XMVECTOR XSTX_DX1_G_UP = {0.0f, 1.0f, 0.0f};
XMVECTORF32 XSTX_DX1_LIGHT_DIR = { -1.0f, -1.0f, 0.3f };

// Initialize the world, view and projection matrices
// As this is global, the viewpoint will persist between consecutive sessions
XMMATRIX XSTX_DX1_G_WORLD = XMMatrixIdentity();
XMMATRIX XSTX_DX1_G_VIEW = XMMatrixLookAtLH(XSTX_DX1_G_EYE, XSTX_DX1_G_LOOK_AT, XSTX_DX1_G_UP);
XMMATRIX XSTX_DX1_G_PROJ = XMMatrixPerspectiveFovLH(XM_PIDIV4, 1.0f, 1.0f, 1000.0f);

struct FrameConstantData
{
    XMMATRIX worldViewProj;
    XMVECTORF32 worldLightVec;
};

struct Vertex3
{
    float pos[3];
};

const Vertex3 XSTX_DX1_TRIANGLE_VERTS[3] =
{
    { 2.5f, -2.5f, 0.0f },
    { -2.5f, -2.5f, 0.0f },
    { 0.0f, 2.5f, 0.0f },
};

const float XSTX_DX1_CLEAR_COLORS[][4] = {
    { 0.071f, 0.04f, 0.561f, 1.0f },
    { 0.671f, 0.04f, 0.161f, 1.0f },
    { 0.021f, 0.54f, 0.561f, 1.0f },
    { 0.41f , 0.41f, 0.41f , 1.0f }
};

Game* const Game::startGame(HostedApplication* hostedApplication,
                            const char* title, const char* wndClassName, 
                            unsigned int width, unsigned int height)
{
    printf("Instantiating a game...\n");
    Game * pGame = new Game();
    pGame->setHostedApplication(hostedApplication);
    pGame->setInitParams(title, wndClassName, width, height);
    pGame->setGameThreadHandle( CreateThread(
        NULL, 0, &GameWindow::gameThreadProc, pGame, 0, pGame->getGameThreadIdAddr()) );
    return pGame;
}

Game::Game()
    : g_d3d_Device(NULL)
    , g_d3d_DeviceContext(NULL)
    , g_renderTargetView(NULL)
    , g_swapChain(NULL)
    , g_backBuffer(NULL)
    , g_vertexShader(NULL)
    , g_pixelShader(NULL)
    , g_meshCount(0)
    , g_inputLayout(NULL)
    , g_constantBuffer(NULL)
    , g_rasterState(NULL)
    , g_clearColorIdx(0)
    , g_audioSystem(NULL)
    , g_masterVoice(NULL)
    , g_SourceVoice(NULL)
    , g_vertexBuffer(NULL)
{
#if APPSTREAM_GAME
    g_screenCapture = NULL;
#endif
}

Game::~Game()
{
}

bool Game::initializeAllResources()
{
    printf("[Game] InitializeAllResources called...\n");
    m_showWindow = true;

    printf("[Game] Registering window class\n");
    // Register a window class
    if (!registerWndClass())
    {
        printf("[Game] Failed to register window class\n");
        return false;
    }

    // Create and show/hide the main window
    m_hWnd = createGameWindow(m_showWindow ? SW_SHOW : SW_HIDE);
    if (!m_hWnd)
    {
        printf("[Game] Failed to create game window\n");
		if (UnregisterClass(m_wndClassName.c_str(), NULL))
		{
			printf("[Game] Successfully un-registered window class...\n");
		} else {
			printf("[Game] Failed to un-register window class. Error code: %ld\n", GetLastError());
		}
        return false;
    }

    m_init_directx = setupDirectX();
	if (!m_init_directx)
	{
		printf("[Game] Failed to set-up directX...\n");
        return false;
	}

    printf("[Game] Setting up audio...\n");
    m_init_audio = setupAudio();
    if ( !m_init_audio)
    {
        printf("[Game] Failed to set-up audio...\n");
        return false;
    }
    return true;
}

bool Game::releaseAllResources()
{
    bool success = true;
    destroyAudio();
    m_init_audio = false;

    printf("[Game] Releasing video resources...\n");
    bool gracefulExit = true;

    printf("[Game] destroying directX...\n");
    destroyDirectX();
    m_init_directx = false;

	// destroy window
    if (m_hWnd)
    {
        if (DestroyWindow(m_hWnd))
		{
			printf("[Game] Successfully destroyed window..\n");
		} else {
			printf("[Game] Failed to destroy window... Error code: %ld\n",GetLastError());
            gracefulExit = false;
		}
    }
	// un-register window class
	if (UnregisterClass(m_wndClassName.c_str(), NULL))
	{
		printf("[Game] Successfully un-registered window class...\n");
	} else {
		printf("[Game] Failed to un-register window class. Error code: %ld\n", GetLastError());
        gracefulExit = false;
	}
    return gracefulExit;
}

bool Game::setupDirectX()
{
    std::printf("[Game] Setting up directX...\n");
    bool success = true;
    try {

        g_vertexBuffer = new ID3D11Buffer*[MAX_MESH_COUNT];
        // look for graphics adapters
        success = success && enumerateAdapters();
        // initialize graphics
        success = success && createGraphics();
        // load triangles etc
        success = success && loadGameResources();
        // Create initial triangle
        success = success && createNewTriangle(false);

        if (success)
        {
            std::printf("[Game] Successfully set up directX...\n");
        }
        else
        {
            std::printf("[Game] Failed initializing directX... releasing resources\n");
            teardownGame();
        }
    }
    catch (...)
    {
        std::printf("[Game] Exception thrown at setupDirectX...\n");
        teardownGame();
        return false;
    }
    return success;
}

bool Game::setupAudio()
{
    std::printf("[Game] Setting up audio...\n");
    bool success = true;
    try {
        // initialize audio
        success = success && createAudio();
        // load background music
        success = success && loadAudioResources();
        // Create initial triangle
        if (success)
        {
            std::printf("[Game] Successfully set up audio...\n");
        }
        else
        {
            std::printf("[Game] Failed setting up audio... releasing resources\n");
            teardownGame();
        }
    } catch(...)
    {
        std::printf("[Game] Exception thrown at setupAudio...\n");
        teardownGame();
        return false;
    }
    return success;
}

void Game::teardownGame()
{
    printf("[Game] TeardownGame called...\n");
    destroyAudio();
    destroyDirectX();
    printf("[Game] Done tearing down game...\n");
}

void Game::destroyDirectX()
{
    printf("[Game] destroyDirectX called..\n");
    // release vertex buffer
    if (g_vertexBuffer != NULL) {
        for (unsigned int i = 0; i < g_meshCount; i++) {
            if (releaseResource(g_vertexBuffer[i])) {
                g_vertexBuffer[i] = NULL;
            }
        }
        delete [] g_vertexBuffer;
        g_vertexBuffer = NULL;
        g_meshCount = 0;
    }
    // release vertex shader
    if (releaseResource(g_vertexShader)) {
        g_vertexShader = NULL;
    }
    // release pixel shader
    if (releaseResource(g_pixelShader)) {
        g_pixelShader = NULL;
    }
    // release input layout
    if (releaseResource(g_inputLayout)) {
        g_inputLayout = NULL;
    }
    // release constant buffer
    if (releaseResource(g_constantBuffer)) {
        g_constantBuffer = NULL;
    }
    // release raster state
    if (releaseResource(g_rasterState)) {
        g_rasterState = NULL;
    }
#ifdef APPSTREAM_GAME
    // release screen capture
    if (g_screenCapture != NULL) {
        delete g_screenCapture;
        g_screenCapture = NULL;
    }
#endif
    // release render target view
    if (releaseResource(g_renderTargetView)) {
        g_renderTargetView = NULL;
    }
    // release back buffer
    if (releaseResource(g_backBuffer)) {
        g_backBuffer = NULL;
    }
    // release swap chain
    if (releaseResource(g_swapChain)) {
        g_swapChain = NULL;
    }
    // release d3d device context
    if (releaseResource(g_d3d_DeviceContext)) {
        g_d3d_DeviceContext = NULL;
    }
    // release d3d device
    if (releaseResource(g_d3d_Device)) {
        g_d3d_Device = NULL;
    }
    printf("[Game] Finished destroyDirectX...\n");
}

void Game::destroyAudio()
{
    printf("[Game] DestroyAudio called...\n");
    try {
        // release source voice
        if (g_SourceVoice != NULL) {
            g_SourceVoice->DestroyVoice();
            g_SourceVoice = NULL;
        }
        // release audio data
        if (XSTX_DX1_G_WAVE_BUFFER.pAudioData != NULL) {
            delete [] XSTX_DX1_G_WAVE_BUFFER.pAudioData;
            XSTX_DX1_G_WAVE_BUFFER.pAudioData = NULL;
        }
        // release master voice
        if (g_masterVoice != NULL) {
            g_masterVoice->DestroyVoice();
            g_masterVoice = NULL;
        }
    } catch(...) {
        printf("[Game] Error while releasing audio resources at destroyAudio...\n");
    }
    if (releaseResource(g_audioSystem)) {
        g_audioSystem = NULL;
    }
    printf("[Game] Finished destroyAudio...\n");
}

bool Game::releaseResource(IUnknown* aResource)
{
    if (aResource != NULL) {
        aResource->Release();
        return true;
    }
    return false;
}

//
//  Enumerate and log available adapters
//
bool Game::enumerateAdapters()
{
    IDXGIFactory1* factory = NULL;

    // Create a DirectX graphics interface factory.
    HRESULT result = CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&factory);
    if (FAILED(result))
    {
        wprintf_s(L"Failed to create DXGI Factory\n");
        return false;
    }

    // Use the factory to enumerate available devices
    unsigned int adapterIndex = 0;
    do {
        IDXGIAdapter1* adapter;
        result = factory->EnumAdapters1(adapterIndex, &adapter);
        if (SUCCEEDED(result))
        {
            DXGI_ADAPTER_DESC1 adapterDesc;
            adapter->GetDesc1(&adapterDesc);

            wprintf_s(L"\nFound adapter %d: %s with result: %d\n", adapterIndex, adapterDesc.Description, result);

            // Enumerate the adapter outputs
            IDXGIOutput* adapterOutput;
            int outputIndex = 0;
            do {
                result = adapter->EnumOutputs(outputIndex, &adapterOutput);
                if (SUCCEEDED(result))
                {
                    DXGI_OUTPUT_DESC adapterOutputDesc;
                    adapterOutput->GetDesc(&adapterOutputDesc);

                    wprintf_s(L"--Found adapterOutput %d: %s with result: %d\n", outputIndex, adapterOutputDesc.DeviceName, result);
                    wprintf_s(L"----Attached to Desktop: %s\n", adapterOutputDesc.AttachedToDesktop ? L"TRUE" : L"FALSE");
                    wprintf_s(L"----DesktopCoordinates: (T) %d (L) %d (B) %d (R) %d\n",
                        adapterOutputDesc.DesktopCoordinates.top,
                        adapterOutputDesc.DesktopCoordinates.left,
                        adapterOutputDesc.DesktopCoordinates.bottom,
                        adapterOutputDesc.DesktopCoordinates.right);
                    wprintf_s(L"----Rotation: %d\n", adapterOutputDesc.Rotation);

                    adapterOutput->Release(); adapterOutput = NULL;
                    outputIndex++;
                }
            } while (result != DXGI_ERROR_NOT_FOUND);

            adapter->Release();
            adapter = NULL;
            adapterIndex++;
        }
    } while (result != DXGI_ERROR_NOT_FOUND);

    // returns true if we found any graphics adapter
    return adapterIndex > 0;
}

bool Game::createGraphics()
{
    // Create a D3D11 device with the default adapter
    D3D_FEATURE_LEVEL d3d_FeatureLevel = (D3D_FEATURE_LEVEL)0;

    DXGI_SWAP_CHAIN_DESC swapDesc;
    swapDesc.BufferCount = 2;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.Width = getWidth();
    swapDesc.BufferDesc.Height = getHeight();
    swapDesc.BufferDesc.RefreshRate.Numerator = 30;
    swapDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;
    swapDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_PROGRESSIVE;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.Flags = 0;
    swapDesc.OutputWindow = getWindowHandle();
    swapDesc.SampleDesc.Count = 1;
    swapDesc.SampleDesc.Quality = 0;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapDesc.Windowed = TRUE;

    //Create the swapChain, D3D device, and device context
    HRESULT result = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE,
                        NULL, 0, NULL, 0, D3D11_SDK_VERSION, &swapDesc, &g_swapChain,
                        &g_d3d_Device, &d3d_FeatureLevel, &g_d3d_DeviceContext);

    // Cache the swap chain's back buffer reference
    g_swapChain->GetBuffer(0, IID_PPV_ARGS(&g_backBuffer));

#if APPSTREAM_GAME

    // Try to initialize screen capture methods.
    // We try them explicitly one by one until we find one that works

    std::printf("Trying DirectX capture method\n");
    g_screenCapture = new DXCapture(g_d3d_Device, g_d3d_DeviceContext, g_backBuffer, getWidth(), getHeight());
    if (!g_screenCapture->supported())
    {
        fprintf(stderr, "Cannot initialize DirectX capture method\n");
        delete g_screenCapture;
        g_screenCapture = NULL;
    }
    else
    {
        std::printf("Using DirectX Capture method\n");
        goto foundCaptureMethod;
    }

    // Also try GDI capture method, but in this case it is redundant because
    // the DirectX capture method ought to succeed all the time
    // Note that the GDI capture method is slower, but more compatible across
    // all kinds of applications.
    std::printf("Trying GDI capture method\n");
    g_screenCapture = new GDICapture(getWindowHandle());
    if (!g_screenCapture->supported())
    {
        fprintf(stderr, "Cannot initialize GDI capture method\n");
        delete g_screenCapture;
        g_screenCapture = NULL;
    }
    else
    {
        std::printf("Using GDI Capture method\n");
        goto foundCaptureMethod;
    }

    // at this point, couldn't find any capture methods
    fprintf(stderr, "Could not initialize any capture methods");
    std::printf("Failed at createGraphics\n");
    return false;

foundCaptureMethod:

#endif

    result = g_d3d_Device->CreateRenderTargetView(g_backBuffer, NULL, &g_renderTargetView);

    // Create a constant buffer for a WVP matrix and a global light
    D3D11_BUFFER_DESC Desc;
    Desc.Usage = D3D11_USAGE_DYNAMIC;
    Desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    Desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    Desc.MiscFlags = 0;
    Desc.ByteWidth = sizeof( FrameConstantData );
    g_d3d_Device->CreateBuffer(&Desc, NULL, &g_constantBuffer);

    // Set the viewport
    g_Viewport.TopLeftX = 0.0f;
    g_Viewport.TopLeftY = 0.0f;
    g_Viewport.Width = static_cast<float>(getWidth());
    g_Viewport.Height = static_cast<float>(getHeight());
    g_Viewport.MinDepth = D3D11_MIN_DEPTH;
    g_Viewport.MaxDepth = D3D11_MAX_DEPTH;
    g_d3d_DeviceContext->RSSetViewports(1, &g_Viewport);

    // Set a default raster state
    D3D11_RASTERIZER_DESC rastDesc;
    rastDesc.CullMode = D3D11_CULL_BACK;
    rastDesc.AntialiasedLineEnable = FALSE;
    rastDesc.DepthBias = 0;
    rastDesc.DepthBiasClamp = 0.0f;
    rastDesc.DepthClipEnable = TRUE;
    rastDesc.SlopeScaledDepthBias = 0.0f;
    rastDesc.ScissorEnable = FALSE;
    rastDesc.MultisampleEnable = FALSE;
    rastDesc.FillMode = D3D11_FILL_SOLID;
    rastDesc.FrontCounterClockwise = FALSE;

    g_d3d_Device->CreateRasterizerState(&rastDesc, &g_rasterState);
    g_d3d_DeviceContext->RSSetState(g_rasterState);

    // succeeded initializing graphcs
    std::printf("Succeeded createGraphics...\n");
    return true;
}

bool Game::createAudio()
{
    // Create XAudio2
    HRESULT result = XAudio2Create(&g_audioSystem);
    if (FAILED(result))
    {
        std::printf("Failed to create XAudio2: 0x%08x\n", result);
        return false;
    }

    // Create the master voice
    result = g_audioSystem->CreateMasteringVoice(&g_masterVoice, 2, 48000, 0, NULL, NULL);
    if (FAILED(result))
    {
        std::printf("Failed to create Mastering voice: 0x%08x\n", result);
        return false;
    }

    return true;
}

bool Game::loadGameResources()
{
    // Load the raw vertex shader bytecode from disk and create a vertex shader with it. 
    DWORD bytesRead; // used to count bytes read from disk
    HRESULT result;
    const DWORD bufferSize = 64 * 1024;
    char rawData[bufferSize];

    // read from disk and create a vertex shader
    HANDLE vsFile = CreateFile("resources/VertexShader.cso",
        GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    bytesRead = 0;
    ReadFile(vsFile, rawData, bufferSize, &bytesRead, NULL);
    CloseHandle(vsFile);
    result = g_d3d_Device->CreateVertexShader(rawData, bytesRead, nullptr, &g_vertexShader);
    if (FAILED(result))
    {
        std::printf("Failed to create vertex shader: 0x%08x\n", result);
        return false;
    }

    // Create vertex input layout
    const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    g_d3d_Device->CreateInputLayout(basicVertexLayoutDesc, ARRAYSIZE(basicVertexLayoutDesc),
                                    rawData, bytesRead, &g_inputLayout);

    // Load the raw pixel shader bytecode from disk and create a pixel shader with it. 
    HANDLE psFile = CreateFile("resources/PixelShader.cso",
        GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    bytesRead = 0;
    ReadFile(psFile, rawData, bufferSize, &bytesRead, NULL);
    CloseHandle(psFile);
    result = g_d3d_Device->CreatePixelShader(rawData, bytesRead, nullptr, &g_pixelShader);
    if (FAILED(result))
    {
        std::printf("Failed to create pixel shader: 0x%08x\n", result);
        return false;
    }
    return true;
}

bool Game::loadAudioResources()
{
    // load audio file from resources sub-folder
    char* strFileName = "resources/Leaves in the Wind_48k.wav";
    HANDLE soundFile = CreateFile(strFileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    std::printf("Loading leaves in the wind 48k.wav file: 0x%08x\n", soundFile);

    DWORD dwChunkSize;
    DWORD dwChunkPosition;

    FindChunk(soundFile, fourccFMT, dwChunkSize, dwChunkPosition );
    ReadChunkData(soundFile, &XSTX_DX1_G_WAVE_DATA, dwChunkSize, dwChunkPosition );

    HRESULT result = g_audioSystem->CreateSourceVoice(&g_SourceVoice, (WAVEFORMATEX*)&XSTX_DX1_G_WAVE_DATA);
    if (FAILED(result))
    {
        fprintf(stderr, "Failed to create source voice: 0x%08x\n", result);
        std::printf("Failed loadAudioResources: create source voice\n");
        CloseHandle(soundFile);
        return false;
    }
    std::printf("Created Source Voice: 0x%08x\n", result);

    // fill out the audio data buffer with the contents of the fourccDATA chunk
    FindChunk(soundFile, fourccDATA, dwChunkSize, dwChunkPosition );
    BYTE * pDataBuffer = new BYTE[dwChunkSize];
    ReadChunkData(soundFile, pDataBuffer, dwChunkSize, dwChunkPosition);
    CloseHandle(soundFile);
    soundFile = INVALID_HANDLE_VALUE;

    std::printf("Read audio data size: %d\n", dwChunkSize);

    XSTX_DX1_G_WAVE_BUFFER.AudioBytes = dwChunkSize;          // buffer containing audio data
    XSTX_DX1_G_WAVE_BUFFER.pAudioData = pDataBuffer;          // size of the audio buffer in bytes
    XSTX_DX1_G_WAVE_BUFFER.LoopCount = XAUDIO2_LOOP_INFINITE; // loop forever
    XSTX_DX1_G_WAVE_BUFFER.Flags = XAUDIO2_END_OF_STREAM;     // tell the source voice not to expect any data after this buffer

    result = g_SourceVoice->SubmitSourceBuffer(&XSTX_DX1_G_WAVE_BUFFER);
    if (FAILED(result))
    {
        fprintf(stderr, "Failed to submit source buffer: 0x%08x\n", result);
        std::printf("Failed loadAudioResources: submit source buffer\n");
        return false;
    }
    std::printf("Submit source buffer: 0x%08x\n", result);
    result = g_SourceVoice->Start();
    if (FAILED(result))
    {
        fprintf(stderr, "Failed to start source voice: 0x%08x\n", result);
        std::printf("Failed loadAudioResources: start source voice\n");
        return false;
    }
    std::printf("start source voice: 0x%08x\n", result);

    return true;
}

void Game::draw(bool const stream)
{
    // Specify the render target we created as the output target
    g_d3d_DeviceContext->OMSetRenderTargets(1, &g_renderTargetView, nullptr);

    // Set raster state
    g_d3d_DeviceContext->RSSetState(g_rasterState);

    // Clear the render target to a solid color
    unsigned int bgColorIndex = g_clearColorIdx % ARRAYSIZE(XSTX_DX1_CLEAR_COLORS);
    g_d3d_DeviceContext->ClearRenderTargetView(
        g_renderTargetView, XSTX_DX1_CLEAR_COLORS[bgColorIndex]);

    // Write the combined WVP matrix and light to the constant buffer
    XMMATRIX worldViewProjection = XSTX_DX1_G_WORLD * XSTX_DX1_G_VIEW * XSTX_DX1_G_PROJ;

    D3D11_MAPPED_SUBRESOURCE cbMap;
    g_d3d_DeviceContext->Map(g_constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &cbMap);
    FrameConstantData* pVSPerObject = (FrameConstantData*)cbMap.pData;
    pVSPerObject->worldViewProj = XMMatrixTranspose(worldViewProjection);
    pVSPerObject->worldLightVec = XSTX_DX1_LIGHT_DIR;
    g_d3d_DeviceContext->Unmap(g_constantBuffer, 0);
    g_d3d_DeviceContext->VSSetConstantBuffers(0, 1, &g_constantBuffer);

    // Set the input layout for the model
    g_d3d_DeviceContext->IASetInputLayout(g_inputLayout);

    // Set the vertex and pixel shader stage state.
    g_d3d_DeviceContext->VSSetShader(g_vertexShader, nullptr, 0);
    g_d3d_DeviceContext->PSSetShader(g_pixelShader, nullptr, 0);

    // Set the vertex buffer, and specify the way it defines geometry
    g_d3d_DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    UINT stride = sizeof(Vertex3);
    UINT offset = 0;

    // Render the meshes..no scene graph, just walking a vector
    for (unsigned int i = 0; i < g_meshCount; i++)
    {
        // Set the vertex buffer
        g_d3d_DeviceContext->IASetVertexBuffers(0, 1, &g_vertexBuffer[i], &stride, &offset);
        // Draw the mesh
        g_d3d_DeviceContext->Draw(ARRAYSIZE(XSTX_DX1_TRIANGLE_VERTS), 0);
    }

    // Present the swapChain
    if (getWindowHandle())
    {
        g_swapChain->Present(0, 0);
    }

#ifdef APPSTREAM_GAME
    if (stream) {
    // This block copies the render target to the CPU for video encoding when running as a remote server
    // Capture the frame using our current capture method
    const unsigned char* theFrame = g_screenCapture->capture();
    if (theFrame != NULL)
    {
        getHostedApplication()->postNewFrame(theFrame,  g_screenCapture->getCapturePixelFormat());
    }
    g_screenCapture->endCapture(); // Done with capture data
    }
#endif
}

bool Game::processMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    // Process messages generated locally
    if ((message == WM_KEYDOWN) || (message == WM_KEYUP))
    {
        processGameInput((char)wParam); // Send an input message from the local window
        return true; // We handled this message
    }
    
    return false; // Didn't handle any other message
}

void Game::processGameInput(unsigned char keyCode)
{
    // Some constants to control camera movement and rotation speed
    const float vectorSpeed = 0.5f;
    const float angleSpeed = 0.05f;

    XMVECTOR camForward = XMVector3Normalize(XSTX_DX1_G_LOOK_AT - XSTX_DX1_G_EYE);
    XMVECTOR camRight = XMVector3Normalize(XMVector3Cross(XSTX_DX1_G_UP, camForward));

    // Bind some keys for simple camera movement and some visible effects
    // A, D, W, S, Q, E - camera movement
    // J, K - cycle the background clear color
    // I, O - add / remove extra model instances in random locations centered around the world origin

    switch (keyCode) {
    case 'A':
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_LOOK_AT - camRight * angleSpeed;
        break;

    case 'D':
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_LOOK_AT + camRight * angleSpeed;
        break;

    case 'W':
        XSTX_DX1_G_EYE = XSTX_DX1_G_EYE + camForward * vectorSpeed;
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_EYE + camForward;
        break;

    case 'S':
        XSTX_DX1_G_EYE = XSTX_DX1_G_EYE - camForward * vectorSpeed;
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_EYE + camForward;
        break;

    case 'Q':
        XSTX_DX1_G_EYE = XSTX_DX1_G_EYE - XSTX_DX1_G_UP * vectorSpeed;
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_EYE + camForward;
        break;

    case 'E':
        XSTX_DX1_G_EYE = XSTX_DX1_G_EYE + XSTX_DX1_G_UP * vectorSpeed;
        XSTX_DX1_G_LOOK_AT = XSTX_DX1_G_EYE + camForward;
        break;

    case 'J':
        g_clearColorIdx++;
        break;

    case 'K':
        g_clearColorIdx--;
        break;

    case 'I':
        // create another triangle if possible
        createNewTriangle(true);
        break;

    case 'O':
        // delete the most recent triangle if it exists
        deleteTriangle();
        break;
    }

    // Update the view matrix
    XSTX_DX1_G_VIEW = XMMatrixLookAtLH(XSTX_DX1_G_EYE, XSTX_DX1_G_LOOK_AT, XSTX_DX1_G_UP);
}

bool Game::handleInput(const XStxInputEvent* event)
{
    // Handle input sent from AppStream clients
    if (event->mType == XSTX_INPUT_EVENT_TYPE_KEYBOARD)
    {
        processGameInput(event->mInfo.mKeyboard.mVirtualKey);
    }
    return true;
}

bool Game::createNewTriangle(bool const randomPosition)
{
    // NOTE: this is not thread-safe obviously..
    if (g_meshCount < MAX_MESH_COUNT)
    {
        // Load a model into another vertex buffer
        D3D11_BUFFER_DESC vertexBufferDesc = {0};
        vertexBufferDesc.ByteWidth = sizeof(Vertex3) * ARRAYSIZE(XSTX_DX1_TRIANGLE_VERTS);
        vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        vertexBufferDesc.CPUAccessFlags = 0;
        vertexBufferDesc.MiscFlags = 0;
        vertexBufferDesc.StructureByteStride = 0;

        D3D11_SUBRESOURCE_DATA vertexBufferData;
        if (randomPosition)
        {
            // Randomize the placement
            Vertex3 newTri[3];
            memcpy(newTri, XSTX_DX1_TRIANGLE_VERTS, sizeof(Vertex3) * ARRAYSIZE(XSTX_DX1_TRIANGLE_VERTS));
            int newOffsetX = rand() % 100 - 50;
            int newOffsetY = rand() % 100 - 50;
            int newOffsetZ = rand() % 100 - 50;

            for (int i = 0; i < 3; i++) {
                newTri[i].pos[0] += newOffsetX;
                newTri[i].pos[1] += newOffsetY;
                newTri[i].pos[2] += newOffsetZ;
            }
            vertexBufferData.pSysMem = &newTri;
        }
        else
        {
            vertexBufferData.pSysMem = &XSTX_DX1_TRIANGLE_VERTS;
        }

        vertexBufferData.SysMemPitch = 0;
        vertexBufferData.SysMemSlicePitch = 0;
        g_d3d_Device->CreateBuffer(&vertexBufferDesc, &vertexBufferData, &g_vertexBuffer[g_meshCount]);

        g_meshCount++;

        // successfully create a new triangle
        return true;
    }
    else
    {
        // already have enough triangles floating around !
        return false;
    }
}

bool Game::deleteTriangle()
{
    // NOTE: again, this is not thread-safe..
    if (g_meshCount > 0) {
        g_meshCount--;
        // Free the vertex buffer
        if (releaseResource(g_vertexBuffer[g_meshCount])) {
            g_vertexBuffer[g_meshCount] = NULL;
        }
        return true;
    }
    return false;
}

// If not an AppStream game, runLocal should be defined to prevent link errors
#ifndef APPSTREAM_GAME 

// Run the game locally without using AppStream by instantiating the game,
// starting it up and waiting for it to complete
int runLocal(int argc, const char* argv[])
{
    std::unique_ptr<Game> game(new Game);
    Game* game = Game::::startGame(NULL,
                    "AppStream Example Game", "WWExampleGameWndClass",
                    mVideoWidth, mVideoHeight);
    while( !game->getInitialized() && !game->isError() )
    {
        Sleep(10); // sleep 10 milliseconds
    }
    if (game->isError() || !game->getInitialized())
    {
        // error in initialization
        printf("There was error initializing game...\n");
    } else {
        printf("Successfully initialized game. Starting game...\n");
        game->startRendering();
        WaitForSingleObject(game->getGameThreadHandle(), INFINITE); // wait for game to finish
    }
    return 0;
}

#endif
