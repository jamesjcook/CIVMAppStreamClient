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

import android.content.Context;
import android.util.AttributeSet;
import android.view.MotionEvent;
import android.view.View;
import android.widget.ImageView;

/**
 * An example button that behaves like a key or a joystick or joypad
 * button: You get an event when it's pressed, and one when it's
 * released.
 */
public class KeyImageButton extends ImageView implements View.OnTouchListener {

    private int mKey = 0;
    private OnPressListener mListener;

    /**
     * A string key. Should be a single character.
     */
    public static final String KEY = "key";

    /**
     * A keycode. Should be a VK_* code from Windows if you have a Windows server,
     * NOT an Android keycode (unless the server is Android and is expecting Android
     * keycodes, of course).
     */
    public static final String KEYCODE = "keycode";

    public interface OnPressListener {
        public void onJoyKeyPress(boolean down, int key);
    }

    public KeyImageButton(Context context) {
        super(context);

    }

    public KeyImageButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        initFromAttributeSet(attrs);
    }

    public KeyImageButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        initFromAttributeSet(attrs);
    }

    private void initFromAttributeSet(AttributeSet attrs) {
        String key = attrs.getAttributeValue(null, KEY);

        if (key != null) mKey = key.codePointAt(0);
        else {
            mKey = attrs.getAttributeIntValue(null, KEYCODE, 0);
        }
    }

    static boolean isDownEvent(MotionEvent event) {
        return event.getActionMasked() == MotionEvent.ACTION_DOWN || event.getActionMasked() == MotionEvent.ACTION_POINTER_DOWN;
    }
    static boolean isUpEvent(MotionEvent event) {
        return event.getActionMasked() == MotionEvent.ACTION_UP || event.getActionMasked() == MotionEvent.ACTION_POINTER_UP;
    }


    boolean mDown = false;
    public void setOnPressListener(OnPressListener opl) {
        mListener = opl;
        setOnTouchListener(this);
    }

    @Override
    public boolean onTouch(View v, MotionEvent event) {
        if (mDown && isUpEvent(event)) {
            mListener.onJoyKeyPress(false, mKey);
            mDown = false;
        } else if (!mDown && isDownEvent(event)) {
            mListener.onJoyKeyPress(true, mKey);
            mDown = true;
        }

        return true;
    }

}
