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

#include <cassert>
#include <stdio.h>
#include <string>

#include "windows.h"
#include "d3d9.h"
#include "d3dx9.h"

#include "XStx/server/XStxServerAPI.h" // AppStream server header
#include "ColorConversions.h" // Colorspace conversion functions

using namespace std;

const wstring WINDOW_CLASS = L"AppStreamExampleWindow";
const wstring WINDOW_TITLE = L"AppStream example server (Standalone)";
const int WINDOW_WIDTH = 1280; // Width of window & AppStream video
const int WINDOW_HEIGHT  = 720; // Height of window & AppStream video

// A macro to safely release DX resources
#define SAFE_RELEASE(x) do { if (x) { x->Release(); x = NULL; } } while (false)

// Convenient constants
#define CLEAR_COLOR D3DCOLOR_ARGB(255, 255, 255, 255)
#define EFFECT_FILENAME L"flatShaded.fx"
#define ROTATION_STEP_SIZE 1.0f
#define MOUSE_ROTATION_SCALE 0.1f

// Our vertex
struct VertexPositionColor
{
    FLOAT x, y, z;
    D3DCOLOR color;
};

// Camera parameters
static D3DXVECTOR3 eye(0.0f, 1.0, 4.0f);
static D3DXVECTOR3 at(0.0f, 1.0, 0.0f);
static D3DXVECTOR3 up(0.0f, 1.0f, 0.0f);

static float g_cameraRotation = -315.800018f;
static float g_cameraDistance = eye.z;

static bool g_leftMouseDown = false;
static float g_lastX = 0.0f;

static bool g_running = false; // Are we running?
static HWND g_hWnd = NULL; // Window handle

static IDirect3D9* g_D3D9 = NULL; // Direct3d9
static IDirect3DDevice9* g_D3DDevice = NULL; // D3D9 device
static D3DDISPLAYMODE g_displayMode = {0}; // Display mode
static D3DPRESENT_PARAMETERS g_D3Dpp = {0}; // Presentation parameters

static ID3DXEffect* g_renderEffect = NULL; // Effect to render our mesh with

static ID3DXMesh* g_mesh = NULL; // Mesh to render

static IDirect3DSurface9* g_backBuffer = NULL; // Pointer to our back buffer
static IDirect3DSurface9* g_memBuffer = NULL; // System memory surface to capture frames

static D3DXMATRIX g_view; // View matrix
static D3DXMATRIX g_projection; // Projection matrix

static XStxChromaSampling g_chromaSamplingType = XSTX_CHROMA_SAMPLING_YUV420;
static bool g_streaming = false; // Are we streaming ?

static XStxServerLibraryHandle g_serverLibraryHandle = {0}; // Server library handle, global STX initialization
static XStxServerManagerHandle g_serverManagerHandle = {0}; // Handle to register main server callbacks under
static XStxServerHandle g_serverHandle = {0}; // An STX handle representing a streaming session

static XStxIServerManagerListener g_serverManagerListener = {0}; // Callbacks for initialization, state management and termination of a server
static XStxIServerListener2 g_serverListener = {0}; // Callbacks to to actually start/stop streaming, handle messages with a connected client
static XStxIVideoSource g_videoSource = {0}; // Callbacks to configure, start & stop video streaming
static XStxIInputSink g_inputSink = {0}; // Callback for receiving input from the client

static XStxRawVideoFrame g_videoFrame = {0}; // A container for planar video data

// Forward declarations of functions used in main

// Window proc. Contains our window message handlers
LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

static bool createWindow(HINSTANCE hInstance = NULL); // Creates a Win32 window
static bool initializeD3D(); // Initializes DirectX (9)
static bool initializeScene(); // Initializes scene data (vertex/index buffers, effects)
static void render(); // Renders one frame and if connected, pushes that frame to AppStream

// Initializes AppStream by creating the appropriate handles and setting up callbacks
static XStxResult initializeAppStream();
// Shuts down AppStream when we are done with it
static XStxResult shutdownAppStream();

// Critical section used to ensure integrity of frame data in the event of session termination
CRITICAL_SECTION g_frameCriticalSection;

int main(int argc, const char* argv[]) // Entry point of the application
{
    MSG msg = {0};
    InitializeCriticalSection(&g_frameCriticalSection);
    XStxResult result;

    int exitCode = ERROR_APP_INIT_FAILURE;

    if (!createWindow()) goto cleanup; // Create the window
    if (!initializeD3D()) goto cleanup; // Initialize Direct3D
    if (!initializeScene()) goto cleanup; // Initialize our scene (a cube)

    // Play music
    PlaySound(L"Leaves in the Wind_48k.wav", NULL, SND_ASYNC | SND_LOOP);

    // Initialize AppStream!
    if ((result = initializeAppStream()) != XSTX_RESULT_OK) goto cleanup;

    exitCode = ERROR_SUCCESS;

    g_running = true;
    while (g_running) // Main message loop
    {
        if (PeekMessage(&msg, g_hWnd, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else // Translate messages if any :(, or render :)
            render();
    }

cleanup:
    SAFE_RELEASE(g_mesh);
    SAFE_RELEASE(g_renderEffect);
    SAFE_RELEASE(g_D3DDevice);
    SAFE_RELEASE(g_D3D9);

	// Shut down AppStream
	result = shutdownAppStream();
	printf("Exit! %s\n", XStxResultGetName(result)); // All good?

    // Miscellaneous cleanup
    DeleteCriticalSection(&g_frameCriticalSection);
    PlaySound(NULL, NULL, SND_ASYNC | SND_LOOP); // Stop music

    return (result != XSTX_RESULT_OK) ? 1 : 0;
}
static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE: // We only care about this message
        g_running = false;
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

static bool createWindow(HINSTANCE hInstance)
{
    WNDCLASS wc      = {0}; 
    wc.lpfnWndProc   = wndProc;
    wc.hInstance     = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_BACKGROUND);
    wc.lpszClassName = WINDOW_CLASS.c_str();
    if( !RegisterClass(&wc) )
        return false;

    RECT rect = {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE); // Ensure the client width/height is WINDOW_WIDTH, WINDOW_HEIGHT
    g_hWnd = CreateWindow(wc.lpszClassName,
        WINDOW_TITLE.c_str(),
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, 0, 0, hInstance, NULL);
    if (!g_hWnd) return false;

    return true;
}
static bool initializeD3D()
{
    g_D3D9 = Direct3DCreate9(D3D_SDK_VERSION);
    if (!g_D3D9) return false;

    UINT32 AdapterOrdinal = 0;
    D3DDEVTYPE DeviceType = D3DDEVTYPE_HAL;
    D3DCAPS9 caps;
    g_D3D9->GetDeviceCaps(AdapterOrdinal, DeviceType, &caps);

    D3DPRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(D3DPRESENT_PARAMETERS));

    params.hDeviceWindow = g_hWnd;
    params.AutoDepthStencilFormat = D3DFMT_D24X8;
    params.BackBufferFormat = D3DFMT_X8R8G8B8; // XRGB pixel format
    params.MultiSampleQuality = D3DMULTISAMPLE_NONE; // No MSAA
    params.MultiSampleType = D3DMULTISAMPLE_NONE;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.Windowed = true; // Windowed mode
    params.PresentationInterval = 0;
    params.BackBufferCount = 1;
    params.BackBufferWidth = WINDOW_WIDTH;
    params.BackBufferHeight = WINDOW_HEIGHT;
    params.EnableAutoDepthStencil = true;
    params.Flags = 2;

    g_D3D9->CreateDevice(0, D3DDEVTYPE_HAL, g_hWnd, 64, &params, &g_D3DDevice); // Create HAL device
    if (!g_D3DDevice) return false;

    // Get back buffer in preparation for frame capture
    g_D3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &g_backBuffer);
    D3DSURFACE_DESC backBufferDesc;
    g_backBuffer->GetDesc(&backBufferDesc);
    // Create system memory surface with same pixel format for readback
    g_D3DDevice->CreateOffscreenPlainSurface(backBufferDesc.Width, backBufferDesc.Height, backBufferDesc.Format, D3DPOOL_SYSTEMMEM, &g_memBuffer, NULL);

    return true;
}
static bool initializeScene()
{
    DWORD dwShaderFlags = D3DXSHADER_NO_PRESHADER;

    if (!SUCCEEDED(D3DXCreateEffectFromFile( // Load our simple effect
        g_D3DDevice,
        EFFECT_FILENAME,
        NULL,
        NULL,
        dwShaderFlags,
        NULL,
        &g_renderEffect,
        NULL)))
        return false;

    LPD3DXBUFFER materials = NULL;
    LPD3DXBUFFER effects = NULL;
    DWORD numMaterials;
    // Load mesh
    if (!SUCCEEDED(D3DXLoadMeshFromX(L"as_001.x", D3DXMESH_SYSTEMMEM, g_D3DDevice, NULL, &materials, &effects, &numMaterials, &g_mesh)))
        return false;

	// We don't care about these
	SAFE_RELEASE(materials);
	SAFE_RELEASE(effects);

    // Set up projection matrix
    D3DXMatrixPerspectiveFovRH(
        &g_projection,
        D3DXToRadian(60),
        (FLOAT)(WINDOW_WIDTH) / (FLOAT)(WINDOW_HEIGHT),
        0.01f,
        100.0f);

    return true;
}

static void render()
{
    g_D3DDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, CLEAR_COLOR, 1.0f, 0);

    // Set up view matrix
    eye.z = g_cameraDistance;
    D3DXMatrixLookAtRH(&g_view,
        &eye,
        &at,
        &up);

    D3DXMATRIX world;
    D3DXMatrixRotationY(&world, D3DXToRadian(g_cameraRotation));

    g_renderEffect->SetTechnique("FlatShaded"); // Set effect params
    g_renderEffect->SetMatrix("World", &world);
    g_renderEffect->SetMatrix("View", &g_view);
    g_renderEffect->SetMatrix("Projection", &g_projection);

    if(SUCCEEDED(g_D3DDevice->BeginScene()))
    {
        UINT passes = 0;
        g_renderEffect->Begin(&passes, 0);

        for (UINT i = 0; i < passes; i++)
        {
            g_renderEffect->BeginPass(i);

            g_mesh->DrawSubset(0);
            g_mesh->DrawSubset(1);

            g_renderEffect->EndPass();
        }
        g_renderEffect->End();

        g_D3DDevice->EndScene();
    }

    if (g_streaming && g_running)
    {
        EnterCriticalSection(&g_frameCriticalSection); // Don't want to be interrupted now

        // Copy back buffer data. Can also use D3DXLoadSurfaceFromSurface if we need to resize/change pixel format
        g_D3DDevice->GetRenderTargetData(g_backBuffer, g_memBuffer);
        D3DLOCKED_RECT lockedRect;
        g_memBuffer->LockRect(&lockedRect, NULL, D3DLOCK_READONLY);
        // Convert to YUV so we can supply it to AppStream
        switch (g_chromaSamplingType)
        {
        case XSTX_CHROMA_SAMPLING_YUV420:
            convertToYUV420((unsigned char*)lockedRect.pBits, WINDOW_WIDTH, WINDOW_HEIGHT, 2, 1, 0, 4, lockedRect.Pitch, g_videoFrame.mPlanes);
            break;
        case XSTX_CHROMA_SAMPLING_YUV444:
            convertToYUV444((unsigned char*)lockedRect.pBits, WINDOW_WIDTH, WINDOW_HEIGHT, 2, 1, 0, 4, lockedRect.Pitch, g_videoFrame.mPlanes);
            break;
        default:
            assert(!"Unknown chroma sampling type"); // Make sure we don't get an unknown chroma sampling type
        }
        g_memBuffer->UnlockRect();

        XStxServerPushVideoFrame(g_serverHandle, &g_videoFrame); // Push the video frame

        LeaveCriticalSection(&g_frameCriticalSection);
    }
    
    g_D3DDevice->Present(NULL, NULL, NULL, NULL); // Regardless of streaming, present locally (on server)
}

static XStxResult getVideoMode(void* context, XStxVideoMode* mode)
{
    // We push frames to AppStream when we want to.
    // AppStream will adapt by dropping frames if we're too fast
    *mode = XSTX_VIDEO_MODE_PUSH_IMMEDIATE;
    return XSTX_RESULT_OK;
}
static XStxResult setFramerate(void* context, double fps)
{
    // TODO: Can use the fps supplied by AppStream to go faster or slower
    return XSTX_RESULT_OK;
}
static XStxResult videoStart(void* context)
{
    EnterCriticalSection(&g_frameCriticalSection); // Don't want to be interrupted now
    g_streaming = true; // We're streaming now
    LeaveCriticalSection(&g_frameCriticalSection);
    
    return XSTX_RESULT_OK;
}
static XStxResult getVideoFrame(void* context, XStxRawVideoFrame** frame)
{
    // This is never called because we chose to push frames, 
    // not have AppStream call us to pull them
    return XSTX_RESULT_OK;
}
static XStxResult videoStop(void* context)
{
    EnterCriticalSection(&g_frameCriticalSection); // Don't want to be interrupted now
	g_streaming = false; // We're not streaming any more
	LeaveCriticalSection(&g_frameCriticalSection);
    
    return XSTX_RESULT_OK;
}

static void initializeVideoFrame(XStxRawVideoFrame& frame, XStxChromaSampling chromaSamplingType) // Utility to set up a raw video frame
{
    g_chromaSamplingType = chromaSamplingType;

    frame.mWidth = WINDOW_WIDTH;
    frame.mHeight = WINDOW_HEIGHT;
    frame.mSize = sizeof(XStxRawVideoFrame);
    frame.mTimestampUs = 0;

    int const yPlaneArea = WINDOW_WIDTH * WINDOW_HEIGHT;
    int const yPlaneStride = WINDOW_WIDTH;
    int const uvPlaneArea = (g_chromaSamplingType == XSTX_CHROMA_SAMPLING_YUV420) ? yPlaneArea / 4 : yPlaneArea; // For U, V in YUV420
    int const uvPlaneStride = (g_chromaSamplingType == XSTX_CHROMA_SAMPLING_YUV420) ? WINDOW_WIDTH / 2 : WINDOW_WIDTH;

    // Allocate Y, U and V planes
    for (int plane = 0; plane < 3; plane++)
    {
        if (frame.mPlanes[plane]) 
        {
            delete[] frame.mPlanes[plane];
            frame.mPlanes[plane] = NULL;
        }
        frame.mPlanes[plane] = new unsigned char[(plane == 0) ? yPlaneArea : uvPlaneArea];
        frame.mStrides[plane] = (plane == 0) ? yPlaneStride : uvPlaneStride;
        frame.mBufferSizes[plane] = (plane == 0) ? yPlaneArea : uvPlaneArea;
    }
}

static XStxResult serverReady(void* context)
{
    return XSTX_RESULT_OK; // Perform post-connection initialization here
}
static XStxResult serverStopped(void* context, XStxStopReason reason)
{
    return XSTX_RESULT_OK; // Perform cleanup here after a single session
}
static XStxResult serverConfigReceived(void* context, const XStxServerConfiguration* config)
{
    // Initialize our frame (or frame pool) here
    initializeVideoFrame(g_videoFrame, config->mChromaSampling);
    return XSTX_RESULT_OK;
}
static XStxResult inputReceived(void* context, const XStxInputEvent* event)
{
    float delta = 0.0f;

    switch (event->mType)
    {
    case XSTX_INPUT_EVENT_TYPE_KEYBOARD:
        if (event->mInfo.mKeyboard.mIsKeyDown)
            switch (event->mInfo.mKeyboard.mVirtualKey)
            {
            case VK_LEFT:
                g_cameraRotation -= ROTATION_STEP_SIZE;
                break;
            case VK_RIGHT:
                g_cameraRotation += ROTATION_STEP_SIZE;
                break;
            }
        break;
    case XSTX_INPUT_EVENT_TYPE_MOUSE:
        if (((event->mInfo.mMouse.mButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) == RI_MOUSE_LEFT_BUTTON_DOWN))
        {
            g_lastX = (float)event->mInfo.mMouse.mLastX;
            g_leftMouseDown = true;
        }
        if (((event->mInfo.mMouse.mButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) == RI_MOUSE_LEFT_BUTTON_UP))
            g_leftMouseDown = false;
        if (g_leftMouseDown && ((event->mInfo.mMouse.mFlags & MOUSE_MOVE_ABSOLUTE) == MOUSE_MOVE_ABSOLUTE))
        {
            delta = (float)(g_lastX - event->mInfo.mMouse.mLastX);
            g_cameraRotation -= (delta * MOUSE_ROTATION_SCALE);

            g_lastX = (float)event->mInfo.mMouse.mLastX;
        }
    }
    
    return XSTX_RESULT_OK;
}

static XStxResult serverInitialize(void* context, const XStxServerHandle server, uint32_t timeout, const char* appContext)
{
    XStxResult result;

    // Our server has been initialized in preparation for a streaming session
    g_serverHandle = server;

    // We support YUV420 and YUV444 streaming
    result = XStxServerAddChromaSamplingOption(g_serverHandle, XSTX_CHROMA_SAMPLING_YUV420);
    if (result != XSTX_RESULT_OK) return result;
    result = XStxServerAddChromaSamplingOption(g_serverHandle, XSTX_CHROMA_SAMPLING_YUV444);
    if (result != XSTX_RESULT_OK) return result;

    // Set up our server callbacks
	ZeroMemory(&g_serverListener, sizeof(XStxIServerListener2));
    g_serverListener.mSize = sizeof(XStxIServerListener2);
    g_serverListener.mReadyFcn = serverReady;
    g_serverListener.mStoppedFcn = serverStopped;
    g_serverListener.mSetConfigurationFcn = serverConfigReceived;
    // Provide AppStream our server listener callbacks
    result = XStxServerSetListener2(g_serverHandle, &g_serverListener);
    if (result != XSTX_RESULT_OK)
        return result;
    
	// Set up video callbacks
	ZeroMemory(&g_videoSource, sizeof(XStxIVideoSource));
    g_videoSource.mSize = sizeof(XStxIVideoSource);
    g_videoSource.mGetModeFcn = getVideoMode;
    g_videoSource.mSetFrameRateFcn = setFramerate;
    g_videoSource.mStartFcn = videoStart;
    g_videoSource.mStopFcn = videoStop;
    g_videoSource.mGetFrameFcn = getVideoFrame;
    // Provide AppStream our video callbacks
    result = XStxServerSetVideoSource(g_serverHandle, &g_videoSource, false);
    if (result != XSTX_RESULT_OK)
        return result;

    // Use default audio capture
	ZeroMemory(&g_inputSink, sizeof(XStxIInputSink));
    g_inputSink.mSize = sizeof(XStxIInputSink);
    g_inputSink.mOnInputFcn = inputReceived;

    result = XStxServerSetInputSink(g_serverHandle, &g_inputSink);
    if (result != XSTX_RESULT_OK)
        return result;

    return XSTX_RESULT_OK;
}
static XStxResult serverSaveState(void* context, XStxServerHandle server, uint32_t timeout, XStxStopReason reason)
{
    return XSTX_RESULT_OK; // If we needed to, we'd save our user's state here
}
static XStxResult serverTerminate(void* context, const XStxServerHandle server, uint32_t timeout, XStxStopReason reason)
{
    EnterCriticalSection(&g_frameCriticalSection); // Make sure we don't interrupt a frame push

    for (int plane = 0; plane < 3; plane++) // Cleanup per-session resources
    {
        delete[] g_videoFrame.mPlanes[plane];
        g_videoFrame.mPlanes[plane] = NULL;
    }

    LeaveCriticalSection(&g_frameCriticalSection);
    
    return XSTX_RESULT_OK;
}

static XStxResult initializeAppStream()
{
    // Main AppStream initialization
    XStxResult result;

    g_serverLibraryHandle = NULL;
    g_serverManagerHandle = NULL;

    // Make sure our compile-time and run-time AppStream versions match
    XStxLibraryVersion libVersion;
    XStxGetLibraryVersion(sizeof(libVersion), &libVersion);
    assert(libVersion.mMajorVersion == XSTX_SERVER_API_VERSION_MAJOR);
    printf("Runtime AppStream version: %s (%d.%d.%d)\n",
        libVersion.mDescription,
        libVersion.mMajorVersion,
        libVersion.mMinorVersion,
        libVersion.mBuild);

    // Create our Server Library handle
    if ((result = XStxServerLibraryCreate(
        XSTX_SERVER_API_VERSION_MAJOR,
        XSTX_SERVER_API_VERSION_MINOR,
        &g_serverLibraryHandle)) != XSTX_RESULT_OK)
        return result;

    // Create our Server Manager
    if ((result = XStxServerLibraryCreateXStxServerManager(
        g_serverLibraryHandle, &g_serverManagerHandle)) != XSTX_RESULT_OK)
        return result;

    // Set up callbacks so we know when a server is initialized or terminated
	ZeroMemory(&g_serverManagerListener, sizeof(XStxIServerManagerListener));
    g_serverManagerListener.mSize = sizeof(XStxIServerManagerListener);
    g_serverManagerListener.mServerInitializeFcn = serverInitialize;
    g_serverManagerListener.mServerSaveStateFcn = serverSaveState;
    g_serverManagerListener.mServerTerminateFcn = serverTerminate;
    
    // Supply our callbacks to AppStream
    if ((result = XStxServerManagerSetListener(
        g_serverManagerHandle,
        &g_serverManagerListener)) != XSTX_RESULT_OK)
        return result;

	// Tell the server to start up
	if ((result = XStxServerManagerStart(
		g_serverManagerHandle)) != XSTX_RESULT_OK)
		return result;

    return XSTX_RESULT_OK;
}
static XStxResult shutdownAppStream()
{
	// Recycle our server manager handle
	XStxResult cleanUpResult = XStxServerManagerRecycle(g_serverManagerHandle);
	if (cleanUpResult != XSTX_RESULT_OK) 
		printf("Failed to recycle ServerManager\n");

	// Recycle our server library handle
	cleanUpResult = XStxServerLibraryRecycle(g_serverLibraryHandle);
	if (cleanUpResult != XSTX_RESULT_OK) 
		printf("Failed to recycle ServerLibrary\n");

	return cleanUpResult;
}