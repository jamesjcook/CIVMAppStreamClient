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

#include <d3d9.h>
#include <D3dx9core.h>
#include <stdint.h>
#include <string>
#include <deque>

#include "MUD/base/TimeVal.h"

class DXUIControl
{
public:
    DXUIControl(int windowWidth,int windowHeight, bool visibility)
        :
    mWindowWidth(windowWidth),
    mWindowHeight(windowHeight),
    mVisible(visibility)
    {
    }

    bool isVisible(){return mVisible;} 
protected:
    int mWindowWidth;
    int mWindowHeight;
    bool mVisible;
};

class DXTextBox : public DXUIControl
{
public:

    DXTextBox(
        int windowWidth,
        int windowHeight,
        DWORD format = DT_LEFT,
        D3DCOLOR color = 0xffffffff,
        bool visibility = true)
        :
        DXUIControl(windowWidth, windowHeight, visibility),
        mFormat(format),
        mColor(color),
        mCountDownTimeMs(0),
        mIsCountingDown(false)
    {
    }

    ~DXTextBox()
    {
    }

    /**
     * Sets the position of the text box that encompasses the text
     * 
     * @param [in] rect Windows coordinate position of text box in window 
     */
    void setPosition(RECT rect, const LPD3DXFONT font)
    {
        mOffsetMatrix = getOffsetMatrix(rect);
        mRealPosition = rect;
        
        D3DXFONT_DESC fontDesc;
        font->GetDesc(&fontDesc);
        int padding = ((mRealPosition.bottom - mRealPosition.top) - fontDesc.Height) / 2;
        mTextPosition.top = mRealPosition.top + padding;
        mTextPosition.left = mRealPosition.left + padding;
        mTextPosition.right = mRealPosition.right - padding;
        mTextPosition.bottom = mRealPosition.bottom;
    }

    /**
     * Set the format of text within text box.
     *
     * @param [in] specifics method of formatting the text.
     *
     */ 
    void setFormat(DWORD format)
    {
        mFormat = format;
    }
    
    /**
     * @return format in which to render text
     */
    DWORD getFormat()
    {
        return mFormat;
    }

    /**
     * Set the text's color
     *
     * @param [in] color the text color
     */
    void setColor(D3DCOLOR color)
    {
        mColor = color;
    }
    
    /**
     * @return text color
     */
    D3DCOLOR getColor()
    {
        return mColor;
    }

    /**
     * Set the text of the text box
     * 
     * @param [in] text a null terminated string. 
     */
    void setText(const char* text)
    {
        mText = text;
    }
    
    /**
     * @return a null terminated string.
     */
    const char* getText()
    {
        return mText.c_str();
    }

    /**
     * @returns the text position of the text.  The text position is positioned relative to its
     *          containing box.
     */ 
    RECT getTextPosition()
    {
        return mTextPosition;
    }

    /**
     * @returns normalized device coordinate of the area the text box should be rendered in.
     */
    D3DXMATRIXA16 getOffsetMatrix()
    {
        return mOffsetMatrix;
    }

    void startCountDown(uint32_t timeOutMs)
    {
        mVisible = true;
        mIsCountingDown = true;
        mCountDownTimeMs = timeOutMs;
        mClock.resetMono();
    }

    void update()
    {
        if(mIsCountingDown)
        {
            //count down done
            if(mCountDownTimeMs < mClock.elapsedMono().toMilliSeconds())
            {
                mVisible = false;
                mIsCountingDown = false;
            }
        }
    }
    
    void show()
    {
        mVisible = true;
    }

    void hide()
    {
        mVisible = false;
        //if currently counting down, thens top the countdown as well
        mIsCountingDown = false;
    }
protected:

    //---------------------------------------------------------
    //==                'Utility function'                 ==
    // matrix helper functions for rendering
    //---------------------------------------------------------
    D3DXMATRIX getTranslationMat(const float dx, const float dy, const float dz) 
    {
        D3DXMATRIX ret;

        D3DXMatrixIdentity(&ret);
        ret(3, 0) = dx;
        ret(3, 1) = dy;
        ret(3, 2) = dz;
        return ret;
    }

    //returns a scale matrix.
    D3DXMATRIX getScaleMat(const float dx, const float dy, const float dz) 
    {
        D3DXMATRIX ret;

        D3DXMatrixIdentity(&ret);
        ret(0, 0) = dx;
        ret(1, 1) = dy;
        ret(2, 2) = dz;
        return ret;
    }

    //convert from Windows coordinate to normalized device coordinate
    /*
    (0,0)------------------------>(1280,0)       (-1,1)---------^--------(1,1)
    |                               |        ->    |            |           |
    |                               |              |----------(0,0)-------->|
    v------------------------------(1280,720)    (-1,-1)________|________(1,-1)
    */
    D3DXMATRIXA16 getOffsetMatrix(RECT a)
    {
        float scaleMatDx = (float)(a.right - a.left) /mWindowWidth;
        float scaleMatDy = (float)(a.bottom - a.top) /mWindowHeight;
        float scaleMatDz = 1.0f;
        D3DXMATRIX scaleMat = getScaleMat(scaleMatDx,scaleMatDy,scaleMatDz);

        float translationMatDx = (float)(a.left + a.right) / mWindowWidth - 1.0f;
        float translationMatDy = 1.0f - (float)(a.top + a.bottom) / mWindowHeight;
        float translationMatDz = 0.0f;
        D3DXMATRIX translationMat = getTranslationMat(
            translationMatDx,
            translationMatDy,
            translationMatDz);
        D3DXMATRIXA16 offsetMat = scaleMat * translationMat;
        return offsetMat;
    }
    
    D3DCOLOR mColor;
    D3DXMATRIXA16 mOffsetMatrix;
    RECT mTextPosition;
    RECT mRealPosition;
    DWORD mFormat;
    std::string mText;
    //if this value is -1
    bool mIsCountingDown;
    int32_t mCountDownTimeMs;
    mud::TimeVal mClock;
};

/**
 * Manages a queue of messages to display to the user.  It will make its best guess
 * on estimating the height of the notification, but will the width is hard coded.
 * Each notification will be displayed for at most 5 seconds, and this is not
 * configurable by the client.
 *
 *
 */
class DXToasterController : public DXUIControl
{
public:
    
    // Default colors
    static const D3DCOLOR INFO_TEXT_COLOR       = 0xffffffff; // white
    static const D3DCOLOR WARNING_TEXT_COLOR    = 0xffffffff; // white
    static const D3DCOLOR ERROR_TEXT_COLOR      = 0xffff0000; // red

    static const size_t MAX_TOAST_WIDTH         = 175;
    static const size_t MAX_TOAST_HEIGHT        = 100;
    static const size_t TOAST_PADDING           = 5;
    static const size_t TEXT_PADDING            = 5;

    static const uint64_t MAX_DISPLAY_DURATION_MS = 5000; // toast should only display for 5 seconds 

    DXToasterController(
        int windowWidth,
        int windowHeight,
        size_t fontHeight,
        size_t fontWidth)
        :
        DXUIControl(windowWidth, windowHeight, true),
        mFontCharHeight(fontHeight)
    {
        maxToastCount = windowHeight / (5 + MAX_TOAST_HEIGHT) - 1;
        mMaxCharPerLine = (MAX_TOAST_WIDTH / fontWidth) + 1;
    }
    
    /**
     * @return whether there's strings to display 
     */
    bool haveContent()
    {
        return mToasts.size() > 0;
    }
    
    /**
     * Draw notifications on screen if there are notifications remaining to be drawn.
     * 
     * @param [in] direct3DDevice
     * @param [in] vertextShaderContant
     * @param [in] font used to render text within box
     * @param [in] vertextShaderHandle handle to matrix
     */
    void draw(LPDIRECT3DDEVICE9 direct3DDevice,
        LPD3DXCONSTANTTABLE vertexShaderConstant,
        LPD3DXFONT font,
        const char* vertextShaderHandle)
    {
        purgeOldToast();

        if (mToasts.empty())
        {
            // render nothing
            return;
        }

        RECT toastPosition;
        toastPosition.bottom = mWindowHeight - TOAST_PADDING * 2;

        toastPosition.right = mWindowWidth - TOAST_PADDING * 2;
        toastPosition.left = toastPosition.right - MAX_TOAST_WIDTH;

        size_t toastCount = mToasts.size();
        for (size_t i = toastCount; i > 0; i--)
        {
            Toast toast = mToasts[i - 1];

            // Update top position based on toast's calcualted max height
            toastPosition.top = (LONG)toastPosition.bottom - (LONG)toast.height;

            toast.textBox.setPosition(toastPosition, font);

            vertexShaderConstant->SetMatrix(
                direct3DDevice,
                vertextShaderHandle,
                &(toast.textBox.getOffsetMatrix())
                );
            direct3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

            RECT textPosition = toastPosition;
            textPosition.top = textPosition.top + TEXT_PADDING;
            textPosition.right = textPosition.right - TEXT_PADDING;
            textPosition.bottom = textPosition.bottom - TEXT_PADDING;
            textPosition.left = textPosition.left + TEXT_PADDING;

            font->DrawText(
                NULL, 
                toast.textBox.getText(), 
                -1, 
                &textPosition,
                toast.textBox.getFormat(),
                toast.textBox.getColor());

            // update position of next toast
            toastPosition.bottom = toastPosition.top - TOAST_PADDING;
        }

    }
    
    /**
     * delete all toasters from queue box
     */
    void clearToasters()
    {
        mToasts.clear();
    }
    
    /**
     * Add a status message in in toaster queue.  All info text will show up as white
     * 
     * @param [in] str string to display info text
     */
    void addInfoText(const char* str)
    {
        addText(str, INFO_TEXT_COLOR);
    }
    
    /**
     * Add a warning message in in toaster queue.  All warning text will show up as white
     * 
     * @param [in] str string to display warning text
     */
    void addWarningText(const char* str)
    {
        addText(str, WARNING_TEXT_COLOR);
    }
    
    /**
     * Add a error message in in toaster queue.  All error text will show up as red
     * 
     * @param [in] str string to display error text
     */
    void addErrorText(const char* str)
    {
        addText(str, ERROR_TEXT_COLOR);
    }
    
    /**
     * Add a toast message in toaster queue to be displayed with specified color
     * 
     * @param [in] str string to display
     * @param [in] color the color of the text
     */
    void addText(const char* str, D3DCOLOR color)
    {
        DXTextBox toast(
            mWindowWidth,
            mWindowHeight,
            DT_LEFT | DT_WORDBREAK | DT_TOP,
            color);

        toast.setText(str);
        size_t height = getToastHeight(str);

        Toast t = Toast(toast, mud::TimeVal::mono(), height);
        mToasts.push_back(t);

        // Toss oldest toast if max capacity is reached
        if (mToasts.size() > maxToastCount)
        {
            mToasts.pop_front();
        }
    }

private:

    struct Toast 
    {
        Toast(const DXTextBox& tb, mud::TimeVal t, size_t h)
            :
            textBox(tb),
            starTime(t),
            height(h)
        {}

        mud::TimeVal starTime;
        DXTextBox textBox;
        size_t height;
    };
    
    /**
     * Remove old toasts if they've timed out.
     */
    void purgeOldToast()
    {
        std::deque<Toast> copy;
        mud::TimeVal currTime = mud::TimeVal::mono();

        while(!mToasts.empty())
        {
            Toast t = mToasts.front();
            mToasts.pop_front();

            uint64_t timePassed = (currTime - t.starTime).toMilliSeconds();

            if (timePassed < MAX_DISPLAY_DURATION_MS)
            {
                copy.push_back(t);
            }
        }

        mToasts = copy;
    }
    
    size_t getToastHeight(const char* s)
    {
        size_t len = strlen(s);
        size_t estimatedLineCount = len/mMaxCharPerLine;
        estimatedLineCount = estimatedLineCount == 0 ? 1 : estimatedLineCount;
        size_t toastHeight = (mFontCharHeight)*estimatedLineCount + TEXT_PADDING*2;

        return toastHeight;
    }

    size_t mFontCharHeight;
    size_t mMaxCharPerLine;

    size_t maxToastCount;
    std::deque<Toast> mToasts;
};

