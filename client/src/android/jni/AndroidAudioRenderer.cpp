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


/**
 * SimpleRenderer implementation
 */

#include "AndroidAudioRenderer.h"
#include "MUD/base/TimeVal.h"

#include "OpenSLAudio.h"

#undef LOG_TAG
#define LOG_TAG "AndroidAudioRenderer"
#include "log.h"

AndroidAudioRenderer::AndroidAudioRenderer(
    mud::FixedSizePool<XStxRawAudioFrame> &framePool,
    XStxClientHandle clientHandle)
    :
      AudioRenderer(framePool, clientHandle),
      mAudioIsPlaying(false),
      mPlaybackPosInFrame(0),
      mUsPerSample(0),
      mDidInit(false),
      mPaused(false),
      mExpectedTimestampUs(0),
      mTimelineIntervalStartUs(0),
      mTimelineGapGraceUs(0),
      mPlaying(false),
      mPlaybackInterval(0),
      mPlaybackGap(0)
{

}

AndroidAudioRenderer::~AndroidAudioRenderer()
{
}

XStxResult AndroidAudioRenderer::start()
{
    if (!mDidInit)
    {
        mUsPerSample = (uint32_t)(1000000.0 / (SAMPLING_RATE * NUM_CHANNELS));
        OpenSLCreate(this);
        mDidInit = true;
    }

    OpenSLSetPlaying(NULL, NULL, true);
    return XSTX_RESULT_OK;
}


XStxRawAudioFrame* AndroidAudioRenderer::popFrame(int delay, int msBuffer)
{
    XStxRawAudioFrame *frame = NULL;
    /**
     *  @todo Check for Android-17 and LOW_LATENCY to see if it will
     * give us a more exact value.
     */

    XStxResult result = XStxGetNextAudioFrame(mClientHandle, &frame, delay, msBuffer);
    if (result != XSTX_RESULT_OK)
    {
        return NULL;
    }
    return frame;
}

void AndroidAudioRenderer::pause(bool pause)
{
    LOGV("AndroidAudioRenderer::pause");
    mPaused = pause;
    OpenSLSetMute(pause);
}

void AndroidAudioRenderer::stop()
{
    LOGV("AndroidAudioRenderer::stop");
    OpenSLShutdown();
    LOGV("AndroidAudioRenderer::stop complete");
}

