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


#include "VideoRenderer.h"

#undef LOG_TAG
#define LOG_TAG "VideoRenderer"
#include "log.h"
#include <assert.h>
#include <math.h>

VideoRenderer::~VideoRenderer()
{
    stop();
}

XStxResult VideoRenderer::postFrame(const XStxRawVideoFrame *frame)
{
    if (frame == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    if(mExiting)
        return XSTX_RESULT_OK;

    mFrame = frame;

    // If the new video frame is a different size than the previous frame,
    // then adjust appropriately.
    if ((frame->mWidth != mSourceWidth) || (frame->mHeight != mSourceHeight))
    {
        setSourceDimensions(frame->mWidth, frame->mHeight);
    }
    requestFrame();

    // wait for rendering to pick up frame
    mSampleLock.waitForSignalAndLock();
    mSampleLock.unlock();

    return XSTX_RESULT_OK;
}

int VideoRenderer::checkQueue()
{
    int frame = 0;
    mSampleLock.lock();
    if (mFrame != NULL && !mExiting)
    {
        render();
        mFrame = NULL;
        frame = 1;
        mFrameValid = true;
        mSampleLock.signal();
    }
    mSampleLock.unlock();

    return frame;
}

void VideoRenderer::setSourceDimensions(uint32_t w, uint32_t h)
{
    mSourceWidth = w;
    mSourceHeight = h;
    
    //Always want to force a rescale when the incoming video size changes
    mScaled = false;

    setScaleAndOffset();
}
void VideoRenderer::setDisplayDimensions(uint32_t w, uint32_t h, bool forceRescale)
{
    mDisplayWidth = w;
    mDisplayHeight = h;

    if (forceRescale)
    {
        mScaled = false;
    }

    setScaleAndOffset();
}

void VideoRenderer::setScaleAndOffset()
{
    if (mScaled)
    {
        return;
    }

    //Make sure we have the width and height of both the local display and the
    // source video
    if (mDisplayWidth && mDisplayHeight && mSourceWidth && mSourceHeight)
    {
        //Get the aspect ratio of the display and the source
        float displayAspect = (float)mDisplayWidth / (float)mDisplayHeight;
        float sourceAspect = (float)mSourceWidth / (float)mSourceHeight;

        mScaled = true;

        //If the source aspect ratio is wider then width determines scale
        if (sourceAspect > displayAspect)
        {
            mScale = (float)mSourceWidth / (float)mDisplayWidth;

            //Width is the display width
            mWidth = mDisplayWidth;

            //Height is scaled appropriately
            mHeight = (uint32_t)((float)mSourceHeight / mScale);
        }
        else
        {
            //Otherwise height determines scale
            mScale = (float)mSourceHeight / (float)mDisplayHeight;

            //Height is the display height
            mHeight = mDisplayHeight;

            //Width is scaled appropriately
            mWidth = (uint32_t)((float)mSourceWidth / mScale);
        }

        //Set the width and height offset based on the size of display and
        // size of rendered stream
        mHeightOffset = (mDisplayHeight - mHeight) / 2;
        mWidthOffset = (mDisplayWidth - mWidth) / 2;
    }
}
