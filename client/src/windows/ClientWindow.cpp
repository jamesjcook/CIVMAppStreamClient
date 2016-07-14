/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights
 * Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may
 * not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 * http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, express or implied. See the License for
 * the specific language governing permissions and limitations under
 * the License.
 *
 */


#include "DES/AppStreamClientFileInput.h"
#include "DES/AppStreamClientFileInput.h"
#include "DES/WinEntitlementPromptDlg.h"

#include "ClientWindow.h"
#include "InputHandling/WindowInputCapture.h"
#include "DirectXRenderer.h"
#include "MUD/base/AStdio.h"

#define IDR_MAINFRAME                   128

/* This class set up a client Window that:
* - has an AppStreamWrapper object to start/end the connection and streaming
*      - has a DirectX renderer object to render the received video frames on the client window
* - is responsible for showing the DES window/getting the EntitlementUrl
* - capture user input(keyboard/mouse) at the client window and send the input back using AppStreamWrapper
*/

/** Constructor */
ClientWindow::ClientWindow()
                    : mWidth(0),
                      mHeight(0),
                      mAppStreamWrapper(NULL),
                      mResizeNeeded(false),
                      mWindowBorderAdjustWidth(0),
                      mWindowBorderAdjustHeight(0),
                      mShouldExit(false),
                      dt("ClientWindow", *this),
                      mShouldPromptForEntitlement(false),
                      mStopDrawing(false),
                      mGraphicsInitialized(false)
{

}

void ClientWindow::init()
{
     //CREATE THE WINDOW Right here:
    if (!createWindowHandle(mWindow))
    {
        printf("ClientWindow.cppERROR: can't create Window.");
    }
}
/** Destructor */
ClientWindow::~ClientWindow()
{
    stop();
    mud::ScopeLock sl(mRunningLock);
}

/**
 * Set width and height
 * @param[in] w width
 * @param[in] h height
 */
void ClientWindow::setDimensions(uint32_t w, uint32_t h)
{
    if (w == 0 || h == 0)
    {
        return;
    }
    mResizeNeeded = (mWidth != 0);
    mWidth = w;
    mHeight = h;
    printf("setDimensions(%u, %u)\n", w, h);
}

/**
 * Full screen mode
 * @param[in] isFullscreen
 */
void ClientWindow::setFullscreen(bool isFullscreen)
{
    mIsFullScreen = isFullscreen;
}

void ClientWindow::setAppStreamWrapper(AppStreamWrapper * appstreamwrapper)
{
    mAppStreamWrapper = appstreamwrapper;
}

/** Blocks for the duration of the window being open. */

HWND ClientWindow::createFullScreenWindow(HINSTANCE hInstance)
{
    const HWND desktopWindow = GetDesktopWindow();
    RECT desktopRect;
    GetWindowRect(desktopWindow, &desktopRect);

    return CreateWindowEx(
        NULL,
        "StxWndClass",
        WINDOW_TITLE,
        WS_EX_TOPMOST | WS_POPUP,
        0,
        0,
        desktopRect.right,
        desktopRect.bottom,
        NULL,
        NULL,
        hInstance,
        this);
}

HWND ClientWindow::createWindowedWindow(HINSTANCE hInstance)
{
    DWORD grfStyle;
    RECT rc;
    grfStyle = WS_OVERLAPPEDWINDOW  | WS_VISIBLE;
    SetRect (&rc, 0, 0, mWidth, mHeight);
    AdjustWindowRect (&rc, grfStyle, FALSE);
    mWindowBorderAdjustWidth = (rc.right - rc.left) - mWidth;
    mWindowBorderAdjustHeight = (rc.bottom - rc.top) - mHeight;

    return CreateWindow(
        "StxWndClass",
        WINDOW_TITLE,
        WS_OVERLAPPEDWINDOW  | WS_VISIBLE,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        rc.right - rc.left ,
        rc.bottom - rc.top,
        NULL,
        NULL,
        hInstance,
        this);
}

//note that this function doesn't belong any class since Windows wants it that way, so
//need to use a global variable of input handler to pass into this function.
extern WindowInputCapture *gInputHandler;
extern ClientWindow *gWindow;

mud::SimpleLock controlCLock;

BOOL WINAPI ConsoleHandlerRoutine(DWORD dwCtrlType)
{
    if (dwCtrlType == CTRL_C_EVENT)
    {
        mud::ScopeLock sl(controlCLock);
        gWindow->stop();

		::PostMessage(gWindow->getWindowHandle(), WM_QUIT, 0, 0);

        return TRUE;
    }

    return FALSE;
}


LRESULT WindowsProcessMessage(
    HWND hwindow,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    if (message == WM_CREATE)
    {
        mud::ScopeLock sl(controlCLock);
        SetConsoleCtrlHandler(ConsoleHandlerRoutine, TRUE);
    }

    if(gWindow != NULL)
    {
        if(gWindow->shouldExit())
        {
            PostQuitMessage(0);
        }
        return gInputHandler->HandleMessage(hwindow, message, wParam, lParam);
    }

    return DefWindowProc(hwindow, message, wParam, lParam);
}


bool ClientWindow::createWindowHandle(HWND & window)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    HICON icon = LoadIcon(hInstance, MAKEINTRESOURCE(IDR_MAINFRAME));
    HCURSOR cursor = LoadCursor(NULL, IDC_ARROW);
    WNDCLASS myClass = {
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC,  //UINT      style;
        (WNDPROC) WindowsProcessMessage,     //WNDPROC   lpfnWndProc;
        0,                                   //int       cbClsExtra;
        sizeof(this),                        //int       cbWndExtra;
        hInstance,                           //HINSTANCE hInstance;
        icon,                                //HICON     hIcon;
        cursor,                              //HCURSOR   hCursor;
        NULL,                                //HBRUSH    hbrBackground;
        NULL,                                //LPCTSTR   lpszMenuName;
        "StxWndClass"                        //LPCTSTR   lpszClassName;
    };

    ATOM wndClass = RegisterClass(&myClass);
    if (!wndClass) {
        return false;
    }

    window = mIsFullScreen ? createFullScreenWindow(hInstance) : createWindowedWindow(hInstance);
    if (window == NULL) {
        return false;
    }
    return true;
}



bool ClientWindow::beginDisplay()
{
    mud::ScopeLock sl(mRunningLock);

    if (mEntitlementUrl.empty())
    {
        WinEntitlementPromptDlg::showModalDialog(
                                        mEntitlementUrl,
                                        mEntitlementInfo.serverUrl.c_str(),
                                        mEntitlementInfo.applicationId.c_str(),
                                        mEntitlementInfo.identityToken.c_str());

        if (mEntitlementUrl.empty())
        {
            // No entitlement URL retrieved
            return false;
        }
    }

    ShowWindow(mWindow, SW_SHOW);
    UpdateWindow(mWindow);
    
    dt.start();

    mAppStreamWrapper->connect(mEntitlementUrl);

    eventLoop();

    return true;
}

bool ClientWindow::isGraphicsInitialized()
{
    mud::ScopeLock sl(mGraphicsInitializedLock);
    return mGraphicsInitialized;
}
/**
 * Rendering loop until exit flag is set
 */
extern DirectXRenderer *gDirectXRenderer;
void ClientWindow::renderLoop()
{
    //d3d device creation needs to be on the same thread that call d3dDevice reset()
    //so this function needs to be here. Since this is a new thread, eventLoop()
    //might be called before this initGraphics function below finishes(a race).
    //so we want to signal eventLoop thread when this initGraphics is done
    mAppStreamWrapper->initGraphics(1280,720);
    {
        mud::ScopeLock sl(mGraphicsInitializedLock);
        mGraphicsInitialized = true;
    }
    while (!shouldStopDrawing())
    {
        //ask the AppStreamWrapper object for the FPS and inform
        //the directX renderer
        char fpsText[6];
        mud::AStdio::snprintf(fpsText, 6, "%.2f", mAppStreamWrapper->getFPS());
        gDirectXRenderer->setTextBoxText("framerate",fpsText);
        mAppStreamWrapper->step();
    }

}

/** exit event loop */
void ClientWindow::stop()
{
    mud::ScopeLock sl(mExitLock);
    mShouldExit = true;
}

bool ClientWindow::shouldExit()
{
    mud::ScopeLock sl(mExitLock);
    return mShouldExit;
}
//--------------------------------------------------------------------

/** exit rendering loop */
void ClientWindow::stopDrawing()
{
    {
        mud::ScopeLock sl(mStopDrawingLock);
        mStopDrawing = true;
    }
    dt.join();
}
bool ClientWindow::shouldStopDrawing()
{
    mud::ScopeLock sl(mStopDrawingLock);
    return mStopDrawing;
}
//--------------------------------------------------------------------


/** event handler loop */
void ClientWindow::eventLoop()
{
    bool clientWindowStopped = false;
    while (!shouldExit() && clientWindowStopped == false)
    {
        if (shouldShowEntitlementPrompt())
        {
            std::string errText;
            getEntitlementPromptErrorText(errText);

            INT_PTR retValue = WinEntitlementPromptDlg::showModalDialog(mEntitlementUrl,
                                    mEntitlementInfo.serverUrl.c_str(),
                                    mEntitlementInfo.applicationId.c_str(),
                                    mEntitlementInfo.identityToken.c_str(),
                                    errText.c_str());

            if (!mEntitlementUrl.empty() && IDOK == retValue)
            {
                //hide status text to start a new streaming 
                if(isGraphicsInitialized())
                    gDirectXRenderer->hideTextBox("status");
                completedEntitlementPrompt();
                mAppStreamWrapper->connect(mEntitlementUrl);
            }
        }

        MSG message;
        int bRet;


        //http://stackoverflow.com/questions/1350262/postquitmessage-wont-close-my-app
        //If hWnd is NULL, GetMessage retrieves messages for any window that belongs
        //to the current thread, and any messages on the current thread's message
        //queue whose hwnd value is NULL
        while(!shouldShowEntitlementPrompt() &&
            ((bRet = GetMessage(&message, NULL, 0, 0)) != 0))
        {
            if(-1 == bRet)
            {
                printf("Unexpected error in ClientWindow::EventLoop"
                    " -- GetMessage returns -1\n");
                clientWindowStopped = true;
                // Handle error and exit
            }
            else
            {
                TranslateMessage(&message);
                DispatchMessage(&message);
            }
        }
        clientWindowStopped = false;
    }
}


