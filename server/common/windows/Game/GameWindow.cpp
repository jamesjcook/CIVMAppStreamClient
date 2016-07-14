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

#include "GameWindow.h"

#define WM_SHUTDOWNGAME WM_USER+1

GameWindow::GameWindow()
    : m_init_directx(false)
    , m_init_audio(false)
    , m_gameThreadHandle(0)
    , m_running(false)
    , m_streaming(false)
    , m_isError(false)
    , m_hWnd(0)
{
    setMaxFrameRate(25); // 25 fps by default
}

GameWindow::~GameWindow()
{
    CloseHandle(m_gameThreadHandle);
}

void GameWindow::setHostedApplication(HostedApplication* hostedApplication)
{
    m_hostedApplication = hostedApplication;
}

void GameWindow::setInitParams(const char* title, const char* wndClassName, 
                               unsigned int width, unsigned int height)
{
    m_width = width;
    m_height = height;
    m_title = title;
    m_wndClassName = wndClassName;
}

LRESULT CALLBACK GameWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CLOSE:
        // Don't close the render window
        return 1;
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
        break;
    }

    return 0;
}

ATOM GameWindow::registerWndClass()
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = GameWndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = NULL;
    wcex.hIcon          = LoadIcon(NULL, MAKEINTRESOURCE(IDI_APPLICATION));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = NULL;
    wcex.lpszClassName  = m_wndClassName.c_str();
    wcex.hIconSm        = NULL;

    return RegisterClassEx(&wcex);
}

HWND GameWindow::createGameWindow(int nCmdShow)
{
    // Adjust the window rect so that the client area is of the size asked for
    // not including the window chrome
    // http://stackoverflow.com/a/4843828/802203
    RECT rect;
    rect.left = 0;
    rect.right = getWidth();
    rect.top = 0;
    rect.bottom = getHeight();
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    
    HWND hWnd = CreateWindow(m_wndClassName.c_str(), m_title.c_str(), 
                             WS_OVERLAPPEDWINDOW, 
                             CW_USEDEFAULT, CW_USEDEFAULT, // Default top and left coordinates
                             rect.right - rect.left, 
                             rect.bottom - rect.top, 
                             NULL, NULL, NULL, NULL);

    if (!hWnd)
	{
		printf("Failed to create window... Error code: %ld\n", GetLastError());
        return FALSE;
	}

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return hWnd;
}


void GameWindow::update()
{
    // Don't do anything by default
}

DWORD WINAPI GameWindow::gameThreadProc(void* param)
{
    GameWindow* pGame = (GameWindow*)(param);

    //===================================
    // create game and initialize resources
    if (pGame->initializeAllResources())
    {
        // successfully initialized game resources
        printf("[GameWindow] Successfully initialized game resources...\n");
    } else {
        // failed to initialize game resources
        printf("[GameWindow] Failed to initialize game resources...\n");
        pGame->m_isError = true;
    }
    //======================================
    // run the game
    bool success = pGame->runGameThread();

    //======================================
    // release game resources
    if ( !pGame->releaseAllResources() )
    {
        printf("[GameWindow] Failed to release game resources...\n");
        pGame->m_isError = true;
    } else {
        printf("[GameWindow] Successfully released game resources...\n");
    }

    return success ? 0 : -1;
}

bool GameWindow::runGameThread()
{
    /**
     * Start of main game loop
     */
    printf("[GameWindow] Starting render loop...\n");
    m_timeLastFrame = TimeVal::ZERO;
    m_running = true;

    // Start running the game loop
    while(m_running)
    {
        if (m_isError)
        {
            printf("[GameWindow] Game is in error state...\n");
            break;
        }
        TimeVal frameStart = TimeVal::mono();

        // Only draw if the window is visible
        // http://msdn.microsoft.com/en-us/library/windows/desktop/ms633530(v=vs.85).aspx
        if (IsWindowVisible(m_hWnd))
        {
            draw(m_streaming);
        }
        update();

        // check the time left till next frame and sleep some if necessary
        TimeVal frameEnd = TimeVal::mono();
        m_timeLastFrame = frameEnd - frameStart;
        float timeToSleepMs = m_minFrameTime - (float)( m_timeLastFrame.toMilliSeconds() );
        if (timeToSleepMs > 0.0f)
        {
            Sleep(static_cast<DWORD>(timeToSleepMs));
        }
    }

    /**
     * End of main game loop
     */
    printf("[GameWindow] Finished render loop...\n");
    m_streaming = false;
    return true;
}

void GameWindow::shutdownGame()
{
    printf("ShutdownGame called. Posting shut-down message\n");
    m_running = false;
}

void GameWindow::setMaxFrameRate(float const maxFrameRate)
{
    m_minFrameTime = 1000.0f / maxFrameRate; // in milliseconds
}

bool GameWindow::handleInput(const XStxInputEvent* event)
{
    // Could also post all input here to the Windows message pump
    // so the game could treat all input like local input
    return true;
}
