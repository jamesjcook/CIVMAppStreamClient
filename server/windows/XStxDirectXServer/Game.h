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

#include "Game/GameWindow.h"
#include "Capture/IScreenCapture.h"
// Capture methods that we can use
#include "Audio/Audio.h"
#include "Capture/GDICapture.h"
#include "Capture/DXCapture.h"
#include <d3d11.h>
#include <stdio.h>
#include <DirectXMath.h>
#include <DXGI.h>

using namespace DirectX;

class Game: public GameWindow
{
private:

    // resource initialization
    bool setupDirectX();
    bool setupAudio();

    // resource de-initialization
    void destroyDirectX();
    void destroyAudio();

    // one-stop call to destroy all resources
    void teardownGame();

    // Methods to initialize different subsystems of the game. All thes methods
    // are invoked in the right sequence by setupGame
    bool enumerateAdapters();
    bool createGraphics();
    bool createAudio();
    bool loadGameResources();
    bool loadAudioResources();

    // helper method
    bool releaseResource(IUnknown* aResource);

    // Common input handler so that local and remote inputs
    // are handled in the same place
    void processGameInput(unsigned char keyCode);
    // add a new triangle to the game
    bool createNewTriangle(bool randomPosition);
    // remove a triangle, if it exists
    bool deleteTriangle();

    //==================================================
    // DirectX states
    ID3D11Device* g_d3d_Device;
    ID3D11DeviceContext* g_d3d_DeviceContext;
    ID3D11RenderTargetView* g_renderTargetView;
    IDXGISwapChain* g_swapChain;

#if APPSTREAM_GAME
    IScreenCapture* g_screenCapture;
#endif

    ID3D11Texture2D* g_backBuffer;
    ID3D11VertexShader* g_vertexShader;
    ID3D11PixelShader* g_pixelShader;
    unsigned int volatile g_meshCount;
    ID3D11Buffer** g_vertexBuffer;
    ID3D11InputLayout* g_inputLayout;
    ID3D11Buffer* g_constantBuffer;

    // raster state
    ID3D11RasterizerState* g_rasterState;

    D3D11_VIEWPORT g_Viewport;

    unsigned int g_clearColorIdx;

    IXAudio2* g_audioSystem;
    IXAudio2MasteringVoice* g_masterVoice;
    IXAudio2SourceVoice* g_SourceVoice;
    //====================================================

public:

    /**
     * @fn  bool Game::startGame();
     *
     * @brief   Instantiates the game and initializes resources. @see GameWindow::update - @see GameWindow::render loop
     *
     * @return  pointer to the instantiated Game object
     */
    static Game* const startGame(
                HostedApplication* app, const char* title, const char* wndClassName,
                unsigned int width, unsigned int height);

    /**
     * @fn  Game::Game()
     *
     * @brief   Default constructor
     *
     */

    Game();

    /**
     * @fn  Game::~Game()
     *
     * @brief   Destructor
     *
     */

    ~Game();

    // initializes all necessary game resources
    bool initializeAllResources();

    // release all resources game has grabbed
    bool releaseAllResources();

    /**
     * @fn  void Game::draw();
     *
     * @brief   Draws one frame of the game
     *
     */
    void draw(bool const stream = false);

    /**
     * @fn  bool Game::processMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
     *
     * @brief   Process any Windows messages sent locally. This game currently handles only
     *          WM_KEYDOWN and WM_KEYUP and routes those key presses to a private method,
     *          processGameInput
     *
     * @param   hWnd    Handle of the window.
     * @param   message The message.
     * @param   wParam  The wParam field of the message.
     * @param   lParam  The lParam field of the message.
     *
     * @return  true if the input was handled, false otherwise
     */

    bool processMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

    /**
     * @fn  bool Game::handleInput(const XStxInputEvent* event);
     *
     * @brief   Routes any remote input sent from an AppStream client to processGameInput
     *          for handling
     *
     * @param   event   The event.
     *
     * @return  true if it succeeds, false if it fails.
     */

    bool handleInput(const XStxInputEvent* event);
};
