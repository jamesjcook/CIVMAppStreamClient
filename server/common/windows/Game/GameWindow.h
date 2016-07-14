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

#pragma once

#include <string>
#include <Windows.h>

#include "MUD/base/TimeVal.h"
#include "../HostedApplication.h"

using namespace std;
using namespace mud;

class GameWindow
{
private:
    // Hide default copy constructor and assignment operator
    GameWindow(GameWindow const&) {}
    GameWindow& operator=(GameWindow const&) {}

public:

    /**
     * Initializes all necessary resources
     */
    virtual bool initializeAllResources() = 0;

    /**
     * Releases all resources game has grabbed
     */
    virtual bool releaseAllResources() = 0;

    /**
     * @fn  void GameWindow::shutdownGame();
     *
     * @brief   Posts a message to the message loop instructing the game to shut down.
     *          Note that at this point, the game has NOT shut down. Use @see GameWindow::getGameThreadHandle
     *          to actually wait for the game loop to terminate
     *
     */
    void shutdownGame();

    /**
     * @fn  void GameWindow::setMaxFrameRate(float maxFrameRate);
     *
     * @brief   Sets maximum frame rate at which the game loop will run. The thread
     *          controlling the game loop sleeps for the remainder of allowed max frame time,
     *          if any is left
     *
     * @param   maxFrameRate    The maximum frame rate to run at
     */
    void setMaxFrameRate(float const maxFrameRate);

    /**
     * @fn  virtual void GameWindow::draw() = 0;
     *
     * @brief   Abstract method that is invoked once per frame from the render loop thread
     *          when it is time to draw the contents of the game window. Note that this method 
     *          is NOT invoked if the window is not visible
     *
     */
    virtual void draw(bool const stream = false) = 0;

    /**
     * @fn  virtual void GameWindow::update();
     *
     * @brief   Virtual method that is invoked once per frame from the render loop thread
     *          when it is time for the game to update its internal state (animations, 
     *          game-state, etc.). The default implementation does nothing, for the case where
     *          @see GameWindow::draw also updates games state
     *
     */

    virtual void update();

    /**
     * @fn  virtual bool GameWindow::handleInput(const XStxInputEvent* event);
     *
     * @brief   This method is invoked by the @see HostedApplication implementation when an
     *          input message is received from the AppStream client.
     *
     * @param   event   The input event sent from the AppStream client
     *
     * @return  true if it succeeds, false if it fails. The result has no bearing on default
     *          handling unlike @see GameWindow::processMessage
     */

    virtual bool handleInput(const XStxInputEvent* event);

    /**
     * @fn  inline const bool GameWindow::getInitialized() const
     *
     * @brief   Gets a flag telling us if the game has been initialized. This does not
     *          mean that the game loop is running, though. See @see GameWindow::getRunning
     *
     * @return  true if it has been initialized, false otherwise.
     */

    inline bool const getInitialized() const { return m_init_directx && m_init_audio; }
    inline bool const isError() const { return m_isError; }
    inline void startRendering() { m_streaming = true; }
    inline bool const getRendering() const { return m_streaming; }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }
    std::string getTitle() const { return m_title; }
    std::string getWndClassName() const { return m_wndClassName; }

protected:
    /**
     * @fn  GameWindow::GameWindow();
     *
     * @brief   Default constructor, initializes a @see GameWindow for use
     *
     */

    GameWindow();

    /**
     * @fn  GameWindow::~GameWindow();
     *
     * @brief   Destructor, cleans up when we're done using this @see GameWindow
     *
     */

    ~GameWindow();

    // Getters to allow sub-classes to read current window state
    HostedApplication* getHostedApplication() const { return m_hostedApplication; }
    HWND getWindowHandle() const { return m_hWnd; }
    void setHostedApplication(HostedApplication* );
    void setInitParams(const char* title, const char* wndClassName, 
                       unsigned int width, unsigned int height);
    inline void setGameThreadHandle(HANDLE handle) { m_gameThreadHandle = handle; }
    inline HANDLE getGameThreadHandle() { return m_gameThreadHandle; }
    inline DWORD* getGameThreadIdAddr() { return &m_gameThreadId; }
    // Helper function to start game threads, param should point at a Game class object 
    static DWORD WINAPI gameThreadProc(void* param);

    ATOM registerWndClass();
    HWND createGameWindow(int nCmdShow);

    bool runGameThread();
    bool processPendingMessages();
 
    // Required to let GameWndProc access internalShutdownGame
    friend LRESULT CALLBACK GameWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    HostedApplication* m_hostedApplication;
    TimeVal m_timeLastFrame;
    float m_minFrameTime;

    // flag indicating whether DirectX resources are properly initialized
    volatile bool m_init_directx;
    // flag indicating whether audio resources are properly initialized
    volatile bool m_init_audio;

    // flag indicating whether game is running
    volatile bool m_running;

    // flag indicating whether game is currently rendering
    volatile bool m_streaming;

    // flag indicating whether there was error in game flow
    volatile bool m_isError;
    
    // Width and height of the window client area
    unsigned int m_width;
    unsigned int m_height;

    string m_title;
    string m_wndClassName;

    bool m_showWindow;

    HANDLE m_gameThreadHandle;
    DWORD m_gameThreadId;

    HWND m_hWnd;
};
