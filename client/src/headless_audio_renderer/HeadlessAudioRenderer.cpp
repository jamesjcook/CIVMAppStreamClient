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

#include "HeadlessAudioRenderer.h"

#undef LOG_TAG
#define LOG_TAG "AudioRenderer"

#include "log.h"

/**
 * The PortAudio based audio renderer.
 */
/** Constructor */
HeadlessAudioRenderer::HeadlessAudioRenderer(
                        mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                        XStxClientHandle clientHandle) 
                            : AudioRenderer(framePool, clientHandle),
                              mTimeoutMarginInMs(INITIAL_TIMEOUT_MARGIN_MS),
                              mAudioIsPlaying(false),
                              mFrame(NULL),
                              mPlaybackPosInFrame(0),
                              mShouldStop(false),
                              mReceivedPackets(0),
                              rt("HeadlessAudioRenderer", *this)
{
}


/** Destructor */
HeadlessAudioRenderer::~HeadlessAudioRenderer()
{
    stop();
}

/**
 * Acquire an audio frame to render.
 */
XStxRawAudioFrame* HeadlessAudioRenderer::popFrame(int delay, int msBuffer)
{
    XStxRawAudioFrame * frame = NULL;

    XStxResult result = XStxGetNextAudioFrame(
                                    mClientHandle, 
                                    &frame, 
                                    delay, 
                                    msBuffer);

    if (result != XSTX_RESULT_OK)
    {
        LOGV("Failed to get audio frame! %d",result);

        return NULL;
    }
    return frame;
}

/**
 * Start an audio stream. After this, audio stream callbacks will
 * start to request audio to play back.
 */
XStxResult HeadlessAudioRenderer::start()
{
    if (!mAudioIsPlaying)  
    {
        mAudioIsPlaying = true;
        rt.start();
    }

    return XSTX_RESULT_OK;
}

/**
 * Pause the audio renderer.
 */
void HeadlessAudioRenderer::pause(bool pause)
{
	LOGV("HeadlessAudioRenderer::pause");
    mPaused = pause;
}

/**
 * Stop the audio renderer.
 */
void HeadlessAudioRenderer::stop()
{
    {
        mud::ScopeLock sl(mStopLock);
        mShouldStop = true;
    }
    rt.join();
}

void HeadlessAudioRenderer::renderLoop()
{
   while(1)
    {
        {
            mud::ScopeLock sl(mStopLock);
            if (mShouldStop) 
            {
                return ;
            }
        }
            XStxRawAudioFrame * frame = popFrame(20,10);
            //this should always happen, frame can't be NULL, if an audio packet
            //arrives late then we should expect a concealment frame here, not NULL
            //but if this is NULL, we will have a problem with the logic below
            if(frame != NULL)
            {
                //start the clock on first packet received
                if(mReceivedPackets == 0)
                   mClock.resetMono();
                //no real render, so just recycle:
                mFramePool.recycleElement(frame);

                int sleepTime = (mReceivedPackets + 1) * 10 - mClock.elapsedMono().toMilliSeconds();
                if (sleepTime > 0)
                    mud::ThreadUtil::sleep(sleepTime);
                mReceivedPackets++;

            }
            
    }
}
