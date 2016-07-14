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


#ifndef _included_AudioModule_h
#define _included_AudioModule_h

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>
#include <MUD/memory/FixedSizePool.h>

class AudioRenderer;  // renderer
class AudioDecoder;   // decoder

/**
 * AudioModule provides audio frames and holds a reference to the decoder
 * and the renderer.
 *
 * Actual audio decoder and audio render used by AudioModule are declared
 * in a file (typically) called AudioPipeline.cpp. That file is defined
 * for each supported platform, and instantiates the appropriate audio
 * decoder and renderer for its associated platform.
 */
class AudioModule
{
public:

    /**
     *  Constructor
     */
    AudioModule() :
        mDecoder(NULL),
        mRenderer(NULL)
    {
        memset(&mStxDecoder, 0, sizeof(mStxDecoder));
        memset(&mStxRenderer, 0, sizeof(mStxRenderer));
        memset(&mStxFrameAllocator, 0, sizeof(mStxFrameAllocator));
    }

    /**
     *  Destructor
     */
    ~AudioModule();

    /**
     * Initialize audio module
     * @param[in] clientHandle handle to XStx client
     */
    bool initialize(XStxClientHandle mClientHandle);

    /**
     * Initializes frame allocator
     * @param[in] maxSize maximum buffer size to follow XStxRawAudioFrame
     */
    XStxResult initAllocator(uint32_t maxSize);

    /**
     * Fetch an audio frame if available
     * @param[in] frameSize currently unused
     * @param[in] reserevedFrame to return pointer to audio frame
     *
     */
    XStxResult getFrame(uint32_t frameSize, XStxRawAudioFrame **reservedFrame);

    /**
     * Recycle audio frame
     * @param[in] frameToRecycle pointer to audio frame being recycled
     */
    XStxResult recycleFrame(XStxRawAudioFrame *frameToRecycle);

    /**
     * Get the contained renderer.
     *
     * @return A pointer to the renderer, or NULL if one isn't bound.
     */
    AudioRenderer* getRenderer() { return mRenderer;}

    /**
     * The the largest size that an audio frame packet can be.
     *
     * @return The largest size in bytes.
     */
    static int getMaxSize() { return mMaxSize; }

    /**
     * Get a pointer to the bound decoder, if one exists.
     *
     * @return A pointer to the audio decoder.
     */
    AudioDecoder* getDecoder() { return mDecoder; }
private:
    /**
     *  decoder
     */
    AudioDecoder *mDecoder;

    /**
     *  renderer
     */
    AudioRenderer *mRenderer;

    /**
     *  pool for audio frames
     */
    mud::FixedSizePool<XStxRawAudioFrame> mFramePool;

    /**
     *  size of the pool, enough to hold 0.5-second audio stream
     */
    static const int NUM_FRAMES_IN_POOL = 50;

    /**
     * The largest size a frame can be.
     */
    static int mMaxSize;

    /**
     * Audio decoder XSTX interface.
     */
    XStxIAudioDecoder mStxDecoder;
    /**
     * Audio renderer XSTX interface.
     */
    XStxIAudioRenderer mStxRenderer;
    /**
     * The allocator for audio frames.
     */
    XStxIRawAudioFrameAllocator mStxFrameAllocator;
};

#endif
