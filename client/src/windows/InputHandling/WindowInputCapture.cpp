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

#include "WindowInputCapture.h"
#include "DirectXRenderer.h"
#include <Windowsx.h>
WindowInputCapture::WindowInputCapture(HWND window, AppStreamWrapper* AppStreamWrapper,
			ClientWindow* clientWindow)
{
	initializeInputCapture(window);
	mAppStreamWrapper = AppStreamWrapper;
	mClientWindow = clientWindow;
	mWindow = window;
}

WindowInputCapture::~WindowInputCapture()
{
}

void WindowInputCapture::setWindowBorderSize(int width, int height)
{
    mWindowBorderWidth = width;
    mWindowBorderHeight = height;
}
/** Handles the Windows messaging pipeline. */
LRESULT WindowInputCapture::HandleMessage(
        HWND window,
        UINT message,
        WPARAM wParam,
        LPARAM lParam)
{

        switch (message) {
            case WM_CLOSE:
                //tell eventLoop to return false
                mClientWindow->stop();
                return 0;
#ifndef USE_RAW_INPUT
            case WM_LBUTTONDOWN:
                mouseChange(wParam, lParam, RI_MOUSE_LEFT_BUTTON_DOWN);
                break;
            case WM_LBUTTONUP:
                mouseChange(wParam, lParam, RI_MOUSE_LEFT_BUTTON_UP);
                break;
            case WM_RBUTTONDOWN:
                mouseChange(wParam, lParam, RI_MOUSE_RIGHT_BUTTON_DOWN);
                break;
            case WM_RBUTTONUP:
                mouseChange(wParam, lParam, RI_MOUSE_RIGHT_BUTTON_UP);
                break;
			case WM_MBUTTONUP:
				mouseChange(wParam, lParam, RI_MOUSE_MIDDLE_BUTTON_UP);
				break;
			case WM_MBUTTONDOWN:
				mouseChange(wParam, lParam, RI_MOUSE_MIDDLE_BUTTON_DOWN);
				break;
			case WM_MOUSEWHEEL:
				mouseWheelChange(wParam, lParam);
				break;
            case WM_MOUSEMOVE:
                mouseMove(wParam, lParam);
                break;
            case WM_KEYDOWN:
                keyChange(wParam, lParam, true);
                break;
            case WM_KEYUP:
                keyChange(wParam, lParam, false);
                break;
            case WM_SYSKEYDOWN:
                keyChange(wParam, lParam, true);
                return TRUE;
            case WM_SYSKEYUP:
                keyChange(wParam, lParam, false);
                return TRUE;

#else
            case WM_INPUT:
                rawInput(wParam, lParam);
                break;
#endif
        }
        return DefWindowProc(window, message, wParam, lParam);
}


bool WindowInputCapture::initializeInputCapture(HWND window)
{
#ifdef USE_RAW_INPUT
        // we want RAW_INPUT events for mouse and keyboard
        // http://msdn.microsoft.com/en-us/library/windows/desktop/ms645546%28v=vs.85%29.aspx
        RAWINPUTDEVICE rawInputDevice[2];
        rawInputDevice[0].usUsagePage = ((USHORT) 0x01);
        rawInputDevice[0].usUsage = ((USHORT) 0x02);
        rawInputDevice[0].dwFlags = RIDEV_INPUTSINK;
        rawInputDevice[0].hwndTarget = window;
        rawInputDevice[1].usUsagePage = ((USHORT) 0x01);
        rawInputDevice[1].usUsage = ((USHORT) 0x06);
        rawInputDevice[1].dwFlags = RIDEV_NOLEGACY;
        rawInputDevice[1].hwndTarget = window;

        if (!RegisterRawInputDevices(rawInputDevice, 2, sizeof(rawInputDevice[0])))
        {
            return false;
        }
#endif // USE_RAW_INPUT
        return true;
}
extern  DirectXRenderer *gDirectXRenderer;
void WindowInputCapture::rescaleMouseInput(LPARAM lParam, int32_t & outX, int32_t & outY)
    {
        // if we send absolute input, we need to rescale it
        RECT desktopRect;
        GetWindowRect(mWindow, &desktopRect);
        float rescaleHeight =  gDirectXRenderer->getLastSetHeight() /
            (float) (desktopRect.bottom - desktopRect.top - mWindowBorderHeight);
        float rescaleWidth =  gDirectXRenderer->getLastSetWidth() /
            (float) (desktopRect.right - desktopRect.left - mWindowBorderWidth);

        outX = (int32_t)(GET_X_LPARAM(lParam) * rescaleWidth + 0.5f); // add 0.5 for rounding
        outY = (int32_t)(GET_Y_LPARAM(lParam) * rescaleHeight + 0.5f);

    }
void WindowInputCapture::rescaleMouseInput(int32_t inX, int32_t inY, int32_t & outX, int32_t & outY)
{
	// if we send absolute input, we need to rescale it
	RECT desktopRect;
	GetWindowRect(mWindow, &desktopRect);
	float rescaleHeight =  gDirectXRenderer->getLastSetHeight() /
		(float) (desktopRect.bottom - desktopRect.top - mWindowBorderHeight);
	float rescaleWidth =  gDirectXRenderer->getLastSetWidth() /
		(float) (desktopRect.right - desktopRect.left - mWindowBorderWidth);

	outX = (int32_t)(inX * rescaleWidth + 0.5f); // add 0.5 for rounding
	outY = (int32_t)(inY * rescaleHeight + 0.5f);
}

    void WindowInputCapture::mouseChange(WPARAM wParam, LPARAM lParam, uint32_t buttonFlags)
    {
        if (!mAppStreamWrapper)
        {
            return;
        }
        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mDeviceId = 0;
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_MOUSE;

        inputEvent.mInfo.mMouse.mFlags = MOUSE_MOVE_ABSOLUTE;

        inputEvent.mInfo.mMouse.mButtonFlags = buttonFlags;
        inputEvent.mInfo.mMouse.mButtons = 0;// not needed
        inputEvent.mInfo.mMouse.mButtonData = 0; // not sending wheel data
        rescaleMouseInput(lParam, inputEvent.mInfo.mMouse.mLastX,
            inputEvent.mInfo.mMouse.mLastY);

        inputEvent.mSize = sizeof(inputEvent);
        mAppStreamWrapper->sendInput(inputEvent);
    }

	void WindowInputCapture::mouseWheelChange(WPARAM wParam, LPARAM lParam)
	{
		if (!mAppStreamWrapper)
		{
			return;
		}
		XStxInputEvent inputEvent;
		inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
		inputEvent.mDeviceId = 0;
		inputEvent.mUserId = 0;
		inputEvent.mType = XSTX_INPUT_EVENT_TYPE_MOUSE;

		inputEvent.mInfo.mMouse.mFlags = MOUSE_MOVE_ABSOLUTE;

		inputEvent.mInfo.mMouse.mButtonFlags = RI_MOUSE_WHEEL;
		inputEvent.mInfo.mMouse.mButtonData = GET_WHEEL_DELTA_WPARAM(wParam);
		inputEvent.mInfo.mMouse.mButtons = 0;// not needed
		rescaleMouseInput(lParam, inputEvent.mInfo.mMouse.mLastX,
			inputEvent.mInfo.mMouse.mLastY);

		inputEvent.mSize = sizeof(inputEvent);
		mAppStreamWrapper->sendInput(inputEvent);
	}


    /** Handles WM_MOUSEMOVE messages.
     */
    void WindowInputCapture::mouseMove(WPARAM wParam, LPARAM lParam)
    {
        if (!mAppStreamWrapper)
        {
            return;
        }

        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mDeviceId = 0;
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_MOUSE;

        inputEvent.mInfo.mMouse.mFlags = MOUSE_MOVE_ABSOLUTE;
        inputEvent.mInfo.mMouse.mButtonFlags = 0;
        inputEvent.mInfo.mMouse.mButtons = 0;// not needed
        inputEvent.mInfo.mMouse.mButtonData = 0; // not sending wheel data
        rescaleMouseInput(lParam, inputEvent.mInfo.mMouse.mLastX,
            inputEvent.mInfo.mMouse.mLastY);

        inputEvent.mSize = sizeof(inputEvent);
        mAppStreamWrapper->sendInput(inputEvent);
    }

    /** Handles WM_KEYDOWN & WM_KEYUP messages. */
    void WindowInputCapture::keyChange(WPARAM wParam, LPARAM lParam, bool isKeyDown)
    {
        if (!mAppStreamWrapper)
        {
            return;
        }
        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mDeviceId = 0;
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_KEYBOARD;
      
        inputEvent.mInfo.mKeyboard.mIsKeyDown = isKeyDown;
        inputEvent.mInfo.mKeyboard.mVirtualKey = (int32_t)wParam;
        inputEvent.mInfo.mKeyboard.mScanCode = (int32_t)lParam;
        
        inputEvent.mSize = sizeof(inputEvent);

        mAppStreamWrapper->sendInput(inputEvent);

    }

    /** Handles WM_INPUT messages, and delegates the keyboard and mouse
     * handling. */
    void WindowInputCapture::rawInput(WPARAM wParam, LPARAM lParam)
    {
        if (!mAppStreamWrapper)
        {
            return;
        }
        uint32_t dwSize = 40;
        static uint8_t inputData[40];

        // from Windows extract the RAWINPUT structure that contains the input event
        GetRawInputData((HRAWINPUT) lParam, RID_INPUT, inputData, &dwSize,
            sizeof(RAWINPUTHEADER));

        // http://msdn.microsoft.com/en-us/library/windows/desktop/ms645562(v=vs.85).aspx
        RAWINPUT* rawInput = (RAWINPUT*) inputData;

        if (rawInput->header.dwType == RIM_TYPEMOUSE) {
            rawMouseInput(rawInput);
            return;
        }
        if (rawInput->header.dwType == RIM_TYPEKEYBOARD) {
            rawKeyboardInput(rawInput);
            return;
        }
    }



    /** Raw mouse input. */
    void WindowInputCapture::rawMouseInput(RAWINPUT * rawInput) {
        RID_DEVICE_INFO deviceInfo;
        memset(&deviceInfo, 0, sizeof(deviceInfo));
        deviceInfo.cbSize = sizeof(RID_DEVICE_INFO);

        GetRawInputDeviceInfo(rawInput->header.hDevice, RIDI_DEVICEINFO, &deviceInfo, NULL);


#ifndef EMULATE_TOUCH // by default we will send mouse events, but you can change this.


        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mDeviceId = (XStxInputDeviceId) deviceInfo.mouse.dwId;
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_MOUSE;

        inputEvent.mInfo.mMouse.mFlags = rawInput->data.mouse.usFlags;
        inputEvent.mInfo.mMouse.mButtonFlags = rawInput->data.mouse.usButtonFlags;
        inputEvent.mInfo.mMouse.mButtons = 0;// not needed
        inputEvent.mInfo.mMouse.mButtonData = rawInput->data.mouse.ulRawButtons;
        inputEvent.mInfo.mMouse.mLastX = rawInput->data.mouse.lLastX;
        inputEvent.mInfo.mMouse.mLastY = rawInput->data.mouse.lLastY;
		rescaleMouseInput(rawInput->data.mouse.lLastX, rawInput->data.mouse.lLastY, 
			inputEvent.mInfo.mMouse.mLastX,
			inputEvent.mInfo.mMouse.mLastY);

        inputEvent.mSize = sizeof(inputEvent);
        mAppStreamWrapper->sendInput(inputEvent);

#else // EMULATE_TOUCH
        if (mIsMouseDownForTouch) {
            mAbsX += static_cast<float>(rawInput->data.mouse.lLastX);
            mAbsY += static_cast<float>(rawInput->data.mouse.lLastY);
        }


        // here we are just going to emulate the mouse as a touch event,
        // just to demonstrate how to use it
        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_TOUCH;

        // we are only sending 1 input
        inputEvent.mInfo.mTouch.mPointerCount = 1;
        inputEvent.mInfo.mTouch.mPointers = new XStxPointer[1];

        // this will uniquely identifier our pointer, so we can determine on the
        //  server which pointer is which over a period of time
        inputEvent.mInfo.mTouch.mPointers[0].mPointerId = deviceInfo.mouse.dwId;
        inputEvent.mInfo.mTouch.mPointers[0].mX = mAbsX;
        inputEvent.mInfo.mTouch.mPointers[0].mY = mAbsY;
        inputEvent.mInfo.mTouch.mPointers[0].mPressure = 1.0f;

        if (!mIsMouseDownForTouch) {
            if ((rawInput->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_DOWN) ==
                    RI_MOUSE_LEFT_BUTTON_DOWN) {
                mIsMouseDownForTouch = true;
                inputEvent.mInfo.mTouch.mPointers[0].mTouchType = XSTX_TOUCH_DOWN;
            } else {
                return;
            }
        } else if ((rawInput->data.mouse.usButtonFlags & RI_MOUSE_LEFT_BUTTON_UP) ==
                    RI_MOUSE_LEFT_BUTTON_UP) {
            mIsMouseDownForTouch = false;
            inputEvent.mInfo.mTouch.mPointers[0].mTouchType = XSTX_TOUCH_UP;
        } else {
            inputEvent.mInfo.mTouch.mPointers[0].mTouchType = XSTX_TOUCH_MOVE;
        }


        inputEvent.mSize = sizeof(inputEvent);
        mAppStreamWrapper->sendInput(inputEvent);
        delete [] inputEvent.mInfo.mTouch.mPointers;
#endif // EMULATE_TOUCH

    }

    /** Raw Keyboard input */
    void WindowInputCapture::rawKeyboardInput(RAWINPUT * rawInput) {
        if (rawInput->data.keyboard.Message != WM_KEYDOWN
            && rawInput->data.keyboard.Message != WM_KEYUP) {
            // not key up or key down
            return;
        }
        XStxInputEvent inputEvent;
        inputEvent.mTimestampUs = mud::TimeVal::mono().toMicroSeconds();
        inputEvent.mDeviceId = 0;
        inputEvent.mUserId = 0;
        inputEvent.mType = XSTX_INPUT_EVENT_TYPE_KEYBOARD;

        inputEvent.mInfo.mKeyboard.mIsKeyDown =
            (rawInput->data.keyboard.Message == WM_KEYDOWN);
        inputEvent.mInfo.mKeyboard.mVirtualKey = rawInput->data.keyboard.VKey;
        inputEvent.mInfo.mKeyboard.mScanCode = rawInput->data.keyboard.MakeCode;

        inputEvent.mSize = sizeof(inputEvent);

        mAppStreamWrapper->sendInput(inputEvent);
    }
