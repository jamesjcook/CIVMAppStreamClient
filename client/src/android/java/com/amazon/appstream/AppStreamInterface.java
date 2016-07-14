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

/**
 * The JNI interface is encapsulated here. The "native"
 * functions are defined in C/C++, and the others are used as
 * hooks back into Java *from* C/C++.
 */
public class AppStreamInterface {
    private static final String TAG = "AppStreamInterface";

    public interface AppStreamListener {
        /**
         * Called when the AppStream has successfully connected to a server.
         */
        public void onConnectSuccess();

        /**
         * Called when an error condition is detected.
         */
        public void onErrorMessage(boolean fatal, String message);

        /**
         * Request new frame.
         */
        public void newFrame();

        /**
         * Reconnecting: Bring up a dialog or otherwise notify the user.
         *
         * @param message  If non-null, then we're reconnecting, and the
         *                 given message should be displayed. If null,
         *                 then we're done reconnecting.
         */
        public void onReconnecting( String message ) ;
    }

    private static AppStreamListener mListener = null;

    public static void setListener(AppStreamListener asl) {
        mListener = asl;
    }

    /**
     * Initialize the library.
     *
     * @return Zero on success.
     */
    public static native int initLibrary();

    /**
     * Set the audio to play.
     *
     * @param play   True to play, false to stop.
     */
    public static native void audioSetPlaying(boolean play);

    /**
     * Send a key to the library.
     *
     * @param key Key to send.
     * @param down True if this is a key-down message.
     */
    public static native void keyPress(int key, boolean down);
    /**
     * Send the current joystick state to the native layer.
     *
     * @param buttons A logical OR of all of the buttons currently active.
     * @param leftTrigger
     *                Left trigger, expressed as 0-32767.
     * @param rightTrigger
     *                Right trigger, expressed as 0-32767.
     * @param thumbLX Left thumb stick x axis, expressed as
     *                -32768-32767, with dead zone scaled based on
     *                GAMEPAD_LEFT_THUMB_DEADZONE.
     * @param thumbLY Left thumb stick y axis, expressed as
     *                -32768-32767, with dead zone scaled based on
     *                 GAMEPAD_LEFT_THUMB_DEADZONE.
     * @param thumbRX Right thumb stick x axis, expressed as
     *                -32768-32767, with dead zone scaled based on
     *                 GAMEPAD_RIGHT_THUMB_DEADZONE.
     * @param thumbRY Right thumb stick y axis, expressed as
     *                -32768-32767, with dead zone scaled based on
     *                 GAMEPAD_RIGHT_THUMB_DEADZONE.
     */
    static native void joystickState(
        int buttons,
        int leftTrigger,
        int rightTrigger,
        int thumbLX,
        int thumbLY,
        int thumbRX,
        int thumbRY);

    /**
     * Try to connect to the server.
     * @param address
     */
    public static native void connect(String address);

    /**
     * Call this function from the OpenGL thread to set up the
     * graphic rendering routines and a surface.
     */
    public static native void setupGraphics(int width, int height);

    /**
     * Step the graphic renderer.
     */
    public static native void step();

    /**
     * Stop the connection and shut down everything.
     */
    public static native void stop();

    /**
     * Pause the app.
     * @param pause True to pause; false to resume.
     */
    public static native void pause(boolean pause);

    /**
     * When showing the keyboard, the offset to the display to use.
     *
     * @param offset Offset to use when keyboard is visible.
     */
    public static native void setKeyboardOffset(int offset);

    /**
     * Turn on the hardware decode path, and pass in a buffer to
     * be used to provide data blocks to the MediaCodec.
     */
    public static native void setHardwareDecoder(HardwareDecoder hd);

    /**
     * Request a new frame from the OpenGL view.
     */
    public static void newFrame() {
        if (mListener != null) {
            mListener.newFrame();
        } else {
            Log.e(TAG, "newFrame() called with no listener.");
        }
    }

    /**
     * A callback to let the listener know that we've successfully
     * connected to the server.
     */
    public static void onConnectSuccess() {
        if (mListener != null) {
            mListener.onConnectSuccess();
        } else {
            Log.e(TAG, "onConnectSuccess called with no listener to receive the result");
        }
    }

    /**
     * A callback to bring up or destroy a "reconnecting" dialog.
     *
     * @param message  If non-null, then we're reconnecting, and the
     *                 given message should be displayed. If null,
     *                 then we're done reconnecting.
     */
    public static void onReconnecting( String message ) {
        if (mListener != null) {
            mListener.onReconnecting(message);
        } else {
            Log.e(TAG, "onReconnecting called with no listener to receive the result");
        }
    }

    /**
     * A callback to let the listener know that the server has
     * returned an error.
     *
     * @param fatal True if this is a fatal error.
     * @param message Message to display to the user.
     */
    public static void errorMessage(boolean fatal, String message) {
        if (mListener != null) {
            mListener.onErrorMessage(fatal, message);
        } else {
            Log.e(TAG, "errorMessage called with no listener to receive the result");
        }
    }

    /**
     * Mouse button 1 down.
     */
    public static final int CET_MOUSE_1_DOWN = 0x01;
    /**
     * Mouse button 1 up.
     */
    public static final int CET_MOUSE_1_UP = 0x02;
    /**
     * Extra flag that indicates the mouse message came from a touch.
     */
    public static final int CET_TOUCH_FLAG = 0x8000;

    /**
     * Mouse event to send to the server.
     *
     * @param x X coordinate of mouse event (or mouse wheel delta).
     * @param y Y coordinate of mouse event.
     * @param flags Mouse flags.
     *
     * @see  CET_MOUSE_1_DOWN
     * @see  CET_MOUSE_1_UP
     * @see  CET_MOUSE_2_DOWN
     * @see  CET_MOUSE_2_UP
     * @see  CET_MOUSE_3_DOWN
     * @see  CET_MOUSE_3_UP
     * @see  CET_MOUSE_WHEEL
     * @see  CET_TOUCH_FLAG
     */
    public static native void mouseEvent(int x, int y, int flags);
}
