/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *      http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

package com.amazon.appstream;

import android.util.Log;
import android.view.InputDevice.MotionRange;
import android.view.MotionEvent;
import android.annotation.TargetApi;
import android.os.Build;

/**
 * This class has some useful constants and methods for
 * interfacing with joysticks. If you don't need joystick
 * support in your app, you can safely ignore all of it.
 */
public class JoystickHelper {
    private static final String TAG = "JoystickHelper";

    ///@{

    /**
     * Gamepad button constant. Used in parameter \c buttons in
     * function joystickState().
     */
    public static final int GAMEPAD_DPAD_UP = 0x0001;
    public static final int GAMEPAD_DPAD_DOWN = 0x0002;
    public static final int GAMEPAD_DPAD_LEFT = 0x0004;
    public static final int GAMEPAD_DPAD_RIGHT = 0x0008;
    public static final int GAMEPAD_START = 0x0010;
    public static final int GAMEPAD_BACK = 0x0020;
    public static final int GAMEPAD_LEFT_THUMB = 0x0040;
    public static final int GAMEPAD_RIGHT_THUMB = 0x0080;
    public static final int GAMEPAD_LEFT_SHOULDER = 0x0100;
    public static final int GAMEPAD_RIGHT_SHOULDER = 0x0200;
    public static final int GAMEPAD_A = 0x1000;
    public static final int GAMEPAD_B = 0x2000;
    public static final int GAMEPAD_X = 0x4000;
    public static final int GAMEPAD_Y = 0x8000;

    ///@}

    /// Default deadzone for left thumb stick.
    public static final int GAMEPAD_LEFT_THUMB_DEADZONE = 7849;
    /// Default deadzone for right thumb stick.
    public static final int GAMEPAD_RIGHT_THUMB_DEADZONE = 8689;
    /// Default deadzone for trigger
    public static final int GAMEPAD_TRIGGER_THRESHOLD = 30;
    
    private static class JoystickParams {
        MotionRange leftTrigger;
        MotionRange rightTrigger;
        MotionRange thumbLX;
        MotionRange thumbLY;
        MotionRange thumbRX;
        MotionRange thumbRY;
    };

    private static JoystickParams mJoystickParams = new JoystickParams();

    private static int joystickScale(float in, MotionRange range, int zone) {
        if (range == null) {
            return 0; // no range means we don't have this control
        }
        float min = range.getMin();
        float max = range.getMax();

        if (in < min) {
            in = min;
        }
        if (in > max) {
            in = max;
        }

        if (Math.abs(in) >= range.getFlat()) {
            if (in >= 0) {
                min = range.getFlat();
                return (int)(((in - min) / (max - min)) * (32767 - zone)) + zone;
            } else {
                max = -range.getFlat();
                return (int)(((in - min) / (max - min)) * (32768 - zone) - 32768.0);
            }
        } else { // scaling up TO the zone; this is the nominally ZERO zone
            min = -range.getFlat();
            max = range.getFlat();
            return (int)(((in - min) / (max - min)) * zone * 2 - zone);
        }
    }

    private static int triggerScale(float in, MotionRange range, int threshold) {
        if (range == null) {
            return 0; // no range means we don't have this control
        }

        float min = range.getMin();
        float max = range.getMax();

        if (in < min) {
            in = min;
        }
        if (in > max) {
            in = max;
        }

        if (in >= range.getFlat()) {
            min = range.getFlat();
            return (int)(((in - min) / (max - min)) * (65535 - threshold) + threshold);
        } else {
            max = range.getFlat();
            return (int)(((in - min) / (max - min)) * (threshold));
        }
    }

    private static int mButtons = 0;
    private static int mLeftTrigger = 0;
    private static int mRightTrigger = 0;
    private static int mThumbLX = 0;
    private static int mThumbLY = 0;
    private static int mThumbRX = 0;
    private static int mThumbRY = 0;
    private static boolean mHatAnalog = false;

    /**
     * Send the current joystick state to the native layer.
     * @param hatY
     * @param hatX
     *
     * @param buttons A logical OR of all of the buttons currently active.
     * @param leftTrigger  Left trigger, expressed as 0-32767.
     * @param rightTrigger Right trigger, expressed as 0-32767.
     * @param thumbLX	   Left thumb stick x axis, expressed as
     *                   -32768-32767.
     * @param thumbLY	   Left thumb stick y axis, expressed as
     *                   -32768-32767.
     * @param thumbRX       Right thumb stick x axis, expressed as
     *                   -32768-32767.
     * @param thumbRY       Right thumb stick y axis, expressed as
     *                   -32768-32767.
     */
    public static void setJoystickState(
        float leftTriggerIn,
        float rightTriggerIn,
        float thumbLXIn,
        float thumbLYIn,
        float thumbRXIn,
        float thumbRYIn,
        float hatX,
        float hatY) {
        mLeftTrigger = triggerScale(leftTriggerIn, mJoystickParams.leftTrigger, GAMEPAD_TRIGGER_THRESHOLD);
        mRightTrigger = triggerScale(rightTriggerIn, mJoystickParams.rightTrigger, GAMEPAD_TRIGGER_THRESHOLD);
        mThumbLX = joystickScale(thumbLXIn, mJoystickParams.thumbLX, GAMEPAD_LEFT_THUMB_DEADZONE);
        mThumbLY = joystickScale(thumbLYIn, mJoystickParams.thumbLY, GAMEPAD_LEFT_THUMB_DEADZONE);
        mThumbRX = joystickScale(thumbRXIn, mJoystickParams.thumbRX, GAMEPAD_RIGHT_THUMB_DEADZONE);
        mThumbRY = joystickScale(thumbRYIn, mJoystickParams.thumbRY, GAMEPAD_RIGHT_THUMB_DEADZONE);

        if (mHatAnalog) {
            if (hatX > 0.5) {
                setButton(GAMEPAD_DPAD_RIGHT, true);
            } else {
                setButton(GAMEPAD_DPAD_RIGHT, false);
                if (hatX < -0.5) {
                    setButton(GAMEPAD_DPAD_LEFT, true);
                } else {
                    setButton(GAMEPAD_DPAD_LEFT, false);
                }
            }
            if (hatY > 0.5) {
                setButton(GAMEPAD_DPAD_DOWN, true);
            } else {
                setButton(GAMEPAD_DPAD_DOWN, false);
                if (hatY < -0.5) {
                    setButton(GAMEPAD_DPAD_UP, true);
                } else {
                    setButton(GAMEPAD_DPAD_UP, false);
                }
            }
        }

        AppStreamInterface.joystickState(mButtons, mLeftTrigger, mRightTrigger, mThumbLX, mThumbLY, mThumbRX, mThumbRY);
    }
    /**
     * Send the current joystick motion ranges.
     *
     * @param leftTrigger
     *               Left trigger range.
     * @param rightTriggerDead
     *               Right trigger range.
     * @param thumbLX Left thumb stick X axis range.
     * @param thumbLY Left thumb stick Y axis range.
     * @param thumbRX Right thumb stick X axis range.
     * @param thumbRY Right thumb stick Y axis range.
     * @param dPad
     */
    @TargetApi(Build.VERSION_CODES.HONEYCOMB_MR1)
    public static void joystickDeadZones(
        MotionRange leftTrigger,
        MotionRange rightTrigger,
        MotionRange thumbLX,
        MotionRange thumbLY,
        MotionRange thumbRX,
        MotionRange thumbRY,
        MotionRange dPad) {
        mJoystickParams.leftTrigger = leftTrigger;
        mJoystickParams.rightTrigger = rightTrigger;
        mJoystickParams.thumbLX = thumbLX;
        mJoystickParams.thumbLY = thumbLY;
        mJoystickParams.thumbRX = thumbRX;
        mJoystickParams.thumbRY = thumbRY;

        if (dPad != null) {
            if (dPad.getAxis() == MotionEvent.AXIS_HAT_X || dPad.getAxis() == MotionEvent.AXIS_HAT_Y) {
                mHatAnalog = true;
            } else {
                Log.w(TAG, "Unknown dpad axis: " + dPad.getAxis());
            }
        }

    }
    
    private static void setButton(int flag, boolean down) {
        if (down) {
            mButtons |= flag;
        } else {
            mButtons &= (~flag);
        }
        AppStreamInterface.joystickState(mButtons,mLeftTrigger,mRightTrigger,mThumbLX,mThumbLY,mThumbRX,mThumbRY);
    }
    
}
