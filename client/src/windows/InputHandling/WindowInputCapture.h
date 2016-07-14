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


#include <windows.h>
#include "AppStreamWrapper.h"
#include "ClientWindow.h"
/**
 * This class captures input from a Window.
 * It needs
 * 		- a pointer to an AppStreamWrapper to send input with
 * 		- a pointer to a ClientWindow so it can tell the ClientWindow to stop its eventLoop
 * 		  when user provides quit input(like hitting X button)
 */
class WindowInputCapture
{
public:
	WindowInputCapture(HWND window, AppStreamWrapper* AppStreamWrapper, ClientWindow *clientWindow);
	~WindowInputCapture();
	LRESULT WindowInputCapture::HandleMessage(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam);
	void setWindowBorderSize(int width, int height);
private:
	bool initializeInputCapture(HWND window);

	/** Rescales absolute mouse position. */
    void rescaleMouseInput(LPARAM lParam, int32_t & outX, int32_t & outY);
	void rescaleMouseInput(int32_t inX, int32_t inY, int32_t & outX, int32_t & outY);

    void mouseChange(WPARAM wParam, LPARAM lParam, uint32_t buttonFlags);

	void mouseWheelChange(WPARAM wParam, LPARAM lParam);

    /** Handles WM_MOUSEMOVE messages.
     */
    void mouseMove(WPARAM wParam, LPARAM lParam);

    /** Handles WM_KEYDOWN & WM_KEYUP messages. */
    void keyChange(WPARAM wParam, LPARAM lParam, bool isKeyDown);

    /** Handles WM_INPUT messages, and delegates the keyboard and mouse
     * handling. */
    void rawInput(WPARAM wParam, LPARAM lParam);

     /** Raw mouse input. */
    void rawMouseInput(RAWINPUT * rawInput);

    /** Raw Keyboard input */
    void rawKeyboardInput(RAWINPUT * rawInput);

    AppStreamWrapper* mAppStreamWrapper;
    ClientWindow* mClientWindow;
    HWND mWindow;

    // for rescaling our mouse when we send absolute movements
    int mWindowBorderWidth;
    int mWindowBorderHeight;
};
