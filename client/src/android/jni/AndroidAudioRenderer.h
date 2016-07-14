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



#ifndef _included_AndroidAudioRenderer_h
#define _included_AndroidAudioRenderer_h

#include "AudioRenderer.h"

#include <queue>
#include <stdint.h>

#include <MUD/memory/FixedSizePool.h>

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>

/**
 * An Android implementation of an audio renderer. Moving forward this
 * class will likely evolve to become an abstract base class for audio
 * rendering, but for now it just includes the Android implementation.
 */
class AndroidAudioRenderer : public AudioRenderer
{

public:

    /**
     *  Constructor
     */
    AndroidAudioRenderer(mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                         XStxClientHandle clientHandle);

    /**
     *  Destructor
     */
    ~AndroidAudioRenderer();

    /**
     * Acquire an audio frame to render.
     *
     * @param[in] msBuffer How much time we can wait for the next frame of
     *                 audio to arrive.
     *
     * @return The frame to render, or NULL if none is available.
     */
    XStxRawAudioFrame* popFrame(int delay, int msBuffer);

    /**
     * Start an audio stream. After this, audio stream callbacks will
     * start to request audio to play back.
     */
    XStxResult start();
    void pause(bool pause);
    void stop();

    /**
     * How many bytes per audio sample.
     */
    static const int BYTES_PER_SAMPLE = 2; // assumes 16 bit samples
private:
    // configuration
    uint32_t mUsPerSample;

    /// Have we been initialized?
    bool mDidInit;

    /// Are we currently poused?
    bool mPaused;

    // buffering and playback
    bool mAudioIsPlaying;

    uint32_t mPlaybackPosInFrame;

    // metrics gathering
    uint64_t mExpectedTimestampUs;
    uint64_t mTimelineIntervalStartUs;
    uint64_t mTimelineGapGraceUs;
    uint64_t mPlaybackInterval;
    uint64_t mPlaybackGap;
    bool mPlaying;

};

#endif //_included_AndroidAudioRenderer_h

