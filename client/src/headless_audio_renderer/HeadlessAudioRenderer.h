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

#ifndef _included_HeadlessAudioRenderer_h
#define _included_HeadlessAudioRenderer_h

#include <queue>
#include <stdint.h>


#include <MUD/memory/FixedSizePool.h>
#include "MUD/memory/ThreadsafeQueue.h"

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>
#include "AudioRenderer.h"
#include "MUD/threading/Thread.h"
#include "MUD/threading/ThreadUtil.h"
#include "MUD/base/TimeVal.h"

/**
 * The PortAudio based audio renderer.
 */
class HeadlessAudioRenderer : public AudioRenderer
{
public:

    /** Constructor */
    HeadlessAudioRenderer(
                mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                XStxClientHandle clientHandle);

    /** Destructor */
    virtual ~HeadlessAudioRenderer();

    /**
     * Acquire an audio frame to render.
     *
     * @param[in] msBuffer How much time we can wait for the next frame of
     *                 audio to arrive.
     *
     * @return The frame to render, or NULL if none is available.
     */
    virtual XStxRawAudioFrame* popFrame(int delay, int msBuffer);

    /**
     * Start an audio stream. After this, audio stream callbacks will
     * start to request audio to play back.
     */
    virtual XStxResult start();

    /**
     * Pause the audio renderer.
     *
     * @param pause  True to pause; false to resume.
     */
    virtual void pause(bool pause);

    /**
     * Stop the audio renderer.
     */
    virtual void stop();

protected:

   
   
    /// Are we currently poused?
    bool mPaused ;

   
    // These values set a time margin between the time that the portaudio renderer
    // needs a frame and the time we allow the SDK to wait for the next packet
    // to arrive. For instance, if we set the mTimeoutMarginInMs to 15, it means
    // that if the portaudio renderer tells us that it needs the next frame 
    // within 40 ms, we'll allow the SDK to wait for at most 25 ms before it 
    // treats the a packet as late and returns a pakcet loss concealment frame.
    // On the one hand we'd like to set this margin as low as possible to
    // give rtp packets time to arrive before we insert concealment.  On the other,
    // we need to provide this margin because we find empirically that if we 
    // don't portaudio doesn't write to the driver in time and we hear ugly
    // audio discontinuities. 
    static const uint32_t INITIAL_TIMEOUT_MARGIN_MS = 15;
    uint32_t mTimeoutMarginInMs;
     
    // constants as defined by headers in XStxClientAPI.h:RenderAudioFrame
    static const uint32_t NUM_CHANNELS = 2;
    static const uint32_t SAMPLING_RATE = 48000;
    static const uint32_t NUM_MS_PER_FRAME = 10;
    static const int BYTES_PER_SAMPLE = 2; // 16 bit samples

    XStxRawAudioFrame *mFrame;
    bool mAudioIsPlaying;
    uint32_t mPlaybackPosInFrame;

    mud::SimpleLock mStopLock;
    bool mShouldStop;
private:
    DEFINE_METHOD_THREAD(RenderThread, HeadlessAudioRenderer, renderLoop);
    RenderThread rt;
    void renderLoop();
    mud::TimeVal mClock;
    uint32_t mReceivedPackets;
};

#endif //_included_HeadlessAudioRenderer_h

