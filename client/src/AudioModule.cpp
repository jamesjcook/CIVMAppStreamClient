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


#include "AudioModule.h"

#ifndef HEADLESS_CLIENT
#include "AudioRenderer.h"
#include "AudioDecoder.h"
#else
#include "headless/AudioRenderer.h"
#include "headless/OpusDecoder.h"
#endif

#include "RawAudioFrameAllocator.h"

// The platform and implementation-specific audio pipeline
#include "AudioPipeline.h"

#undef LOG_TAG
#define LOG_TAG "AudioModule"
#include "log.h"


#include <MUD/base/SmartPointers.h>

int AudioModule::mMaxSize = 0;


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Audio Decoder C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
XStxResult decoderStart(void *context)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    AudioModule *am = (AudioModule *)context;
    return am->getDecoder()->start();
}

XStxResult decoderDecodeFrame(void *context, XStxEncodedAudioFrame *enc,
                              XStxRawAudioFrame *dec)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    if (getenv("XSTX_DEBUG_TIMESTAMP"))
    {
        LOGD("Client Audio Timestamp: %llu", enc->mTimestampUs);
    }
    AudioModule *am = (AudioModule *)context;
    return am->getDecoder()->decodeFrame(enc, dec);
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Audio Render C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
XStxResult audioRenderStart(void *context)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((AudioRenderer *)context)->start();
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Audio Allocator C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
XStxResult allocatorInit(void *context, uint32_t maxSize)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((AudioModule *)context)->initAllocator(maxSize);
}
XStxResult allocatorGetFrame(void *context, uint32_t frameSize,
                             XStxRawAudioFrame **reservedFrame)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((AudioModule *)context)->getFrame(frameSize, reservedFrame);
}
XStxResult allocatorRecycleFrame(void *context,
                                 XStxRawAudioFrame *frameToRecycle)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((AudioModule *)context)->recycleFrame(frameToRecycle);
}



//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Audio Module Code
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

/**
 * Initializes frame allocator
 * @param[in] maxSize maximum buffer size to follow XStxRawAudioFrame
 */
XStxResult AudioModule::initAllocator(uint32_t maxSize)
{
    LOGV("AudioModule::initAllocator: %u", maxSize);

    // instantiate allocator
    shared_ptr<RawAudioFrameAllocator>
        allocator(new RawAudioFrameAllocator(maxSize));

    // allocate fixed size pool for audio frames
    if (mFramePool.allocate(allocator, NUM_FRAMES_IN_POOL) != SIMPLE_RESULT_OK)
    {
        // failed to allocate memory
        return XSTX_RESULT_OUT_OF_MEMORY;
    }

    mMaxSize = maxSize;

    // successfully initialized allocator
    return XSTX_RESULT_OK;
}

/**
 * Fetch an audio frame if available
 * @param[in] frameSize currently unused
 * @param[in] reserevedFrame to return pointer to audio frame
 */
XStxResult AudioModule::getFrame(uint32_t frameSize,
                                 XStxRawAudioFrame **reservedFrame)
{
    XStxRawAudioFrame *outFrame = NULL;
    // fetch a frame from frame pool
    if (mFramePool.getElement(outFrame) != SIMPLE_RESULT_OK)
    {
        LOGV("AudioModule::no frames available");
        // no frame is available
        return XSTX_RESULT_FRAME_NOT_AVAILABLE;
    }

    // sanity check the frame we received
    assert(outFrame->mBufferSize >= frameSize);

    // successfully fetched a frame
    *reservedFrame = outFrame;
    return XSTX_RESULT_OK;
}

/**
 * Recycle audio frame
 * @param[in] frameToRecycle pointer to audio frame being recycled
 */
XStxResult AudioModule::recycleFrame(XStxRawAudioFrame *frameToRecycle)
{
    // recycle it back to frame pool
    if (mFramePool.recycleElement(frameToRecycle) != SIMPLE_RESULT_OK)
    {
        // doesn't belong to this pool
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    // successfully recycled
    return XSTX_RESULT_OK;
}

/**
 *  Destructor
 */
AudioModule::~AudioModule()
{
    LOGV("AudioModule::~AudioModule");
    delete mDecoder;
    delete mRenderer;

    mDecoder = NULL;
    mRenderer = NULL;
}

/**
 * Initialize audio module
 * @param[in] clientHandle handle to XStx client
 */
bool AudioModule::initialize(XStxClientHandle clientHandle)
{
    LOGV("AudioModule::initialize");

    // initialize frame allocator
    mStxFrameAllocator.mInitFcn = &allocatorInit;
    mStxFrameAllocator.mInitCtx = this;
    mStxFrameAllocator.mGetAudioFrameBufferFcn = &allocatorGetFrame;
    mStxFrameAllocator.mGetAudioFrameBufferCtx = this;
    mStxFrameAllocator.mRecycleAudioFrameBufferFcn = &allocatorRecycleFrame;
    mStxFrameAllocator.mRecycleAudioFrameBufferCtx = this;
    mStxFrameAllocator.mSize = sizeof(mStxFrameAllocator);
    if (XStxClientSetAudioFrameAllocator(clientHandle, &mStxFrameAllocator)
        != XSTX_RESULT_OK)
    {
        LOGE("Failed to SetAudioFrameAllocator\n");
        return false;
    }

    // initialize decoder
    mDecoder = newAudioDecoder();
    mStxDecoder.mStartFcn = &decoderStart;
    mStxDecoder.mStartCtx = this;
    mStxDecoder.mDecodeAudioFrameFcn = &decoderDecodeFrame;
    mStxDecoder.mDecodeAudioFrameCtx = this;
    mStxDecoder.mSize = sizeof(mStxDecoder);
    if (XStxClientSetAudioDecoder(clientHandle, &mStxDecoder)
        != XSTX_RESULT_OK)
    {
        LOGE("Failed to SetAudioDecoder\n");
        return false;
    }

    // initialize renderer
    mRenderer = newAudioRenderer(mFramePool, clientHandle);

    // Set XStx callbacks and contexts on for the XStxIAudioRenderer struct
    mStxRenderer.mStartFcn = &audioRenderStart;
    mStxRenderer.mStartCtx = mRenderer;
    mStxRenderer.mSize = sizeof(mStxRenderer);

    // set the XStxIAudioRenderer to be used with the given clientHandle
    if (XStxClientSetAudioRenderer(clientHandle, &mStxRenderer)
        != XSTX_RESULT_OK)
    {
        LOGE("Failed to SetAudioRenderer\n");
        return false;
    }
    return true;
}


