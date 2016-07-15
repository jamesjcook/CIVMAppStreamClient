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

/* Dummy audio render duplicates port audio render but does nothing at all, Probably would have been cleaner to just inherrit port audio rather than branch it, but we'll do this for a first pass.*/

#ifndef _included_DummyAudioRenderer_h
#define _included_DummyAudioRenderer_h

#include <queue>
#include <stdint.h>


#include <MUD/memory/FixedSizePool.h>
#include "MUD/memory/ThreadsafeQueue.h"

#include <portaudio.h>

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>

#include "AudioRenderer.h"

/**
 * The DummyAudio based audio renderer.
 */
class DummyAudioRenderer : public AudioRenderer
{
public:

    /** Constructor */
    DummyAudioRenderer(
                mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                XStxClientHandle clientHandle);

    /** Destructor */
    virtual ~DummyAudioRenderer();

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

    /**
    * Initialize DummyAudio stream
    * @return TRUE if successful, FALSE otherwise
    */
    bool initializeDummyAudio();

    /**
     * The fillPABuffer method writes PCM samples to the buffer provided
     * by DummyAudio. DummyAudio specifies how many samples it wants. If
     * that number is not available, fillPABuffer writes zeros. If a
     * number of zeros were written to the DummyAudio callback's output
     * buffer in an earlier fillPABuffer call, it needs to drop that many
     * samples when they become available to prevent the injected zeros
     * from adding latency. We should report dropped audio as a real-time
     * metric, though, and potentially grow latency if we need to by not
     * dropping the samples.
     *
     * This function first drops any PCM samples that are arriving late
     * and have already been replaced with zeros. Then it fills the
     * callback's output buffer with as many requested samples as are
     * available. If the number of samples requested is more than the
     * number available, zeros are written.  The method keeps track of how
     * many zeros are written so that number can be dropped when they
     * become available.
     */
    bool fillPABuffer(
        int16_t* buff, 
        int numSamplesPerChannel,
        const PaStreamCallbackTimeInfo* timeInfo); 

    /** DummyAudio callback */
    static int paStreamCallback(
        const void *inputBuffer, 
        void *outputBuffer,
        unsigned long framesPerBuffer,
        const PaStreamCallbackTimeInfo* timeInfo,
        PaStreamCallbackFlags statusFlags,
        void *userData );

    // actual playback
    PaStream *mDummyAudioStream;

    // flag indicating initialization status of DummyAudio
    bool mDidInit;
   
    /// Are we currently poused?
    bool mPaused ;

    static const int SUGGESTED_PA_LATENCY_MS = 40; // in ms.

    // These values set a time margin between the time that the DummyAudio renderer
    // needs a frame and the time we allow the SDK to wait for the next packet
    // to arrive. For instance, if we set the mTimeoutMarginInMs to 15, it means
    // that if the DummyAudio renderer tells us that it needs the next frame 
    // within 40 ms, we'll allow the SDK to wait for at most 25 ms before it 
    // treats the a packet as late and returns a pakcet loss concealment frame.
    // On the one hand we'd like to set this margin as low as possible to
    // give rtp packets time to arrive before we insert concealment.  On the other,
    // we need to provide this margin because we find empirically that if we 
    // don't DummyAudio doesn't write to the driver in time and we hear ugly
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
};

#endif //_included_DummyAudioRenderer_h

