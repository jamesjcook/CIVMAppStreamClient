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


#include "VideoModule.h"
#include "VideoPipeline.h"

#include <MUD/base/TimeVal.h>
#include <assert.h>

#undef LOG_TAG
#define LOG_TAG "VideoModule"
#include "log.h"


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Decoder C callbacks
//
//          Also see pipelineVideoDecoderGetCapabilities() in
//          VideoPipeline.cpp.
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
XStxResult videoDecoderStart(void *context)
{
    LOGV("videoDecoderStart");
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((VideoDecoder *)context)->init();
}
XStxResult decodeFrame(void *context, XStxEncodedVideoFrame *enc,
                       XStxRawVideoFrame *dec)
{
    uint64_t startTime = mud::TimeVal::mono().toMilliSeconds();

    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    if (getenv("XSTX_DEBUG_TIMESTAMP"))
    {
        LOGD("Client Video Timestamp: %llu", enc->mTimestampUs);
    }

    XStxResult result = ((VideoDecoder *)context)->decodeFrame(enc, dec);

    uint64_t finishTime = mud::TimeVal::mono().toMilliSeconds();
    uint32_t totalTime = (uint32_t)(finishTime - startTime);

    // We're trying for 30FPS, so anything greater than 33ms is a problem.
    // Logging things that are close
    if (totalTime > 30)
    {
//        LOGW("decodeTime: %dms", totalTime);
    }

    return result;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Renderer C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//


XStxResult renderFrame(void *context, XStxRawVideoFrame *frame)
{

    if (context == NULL || frame == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((VideoRenderer *)context)->postFrame(frame);
}


XStxResult rendererMaxResolution(void *context, uint32_t maxWidth,
                                 uint32_t maxHeight)
{
    LOGV("rendererMaxResolution %d,%d", maxWidth, maxHeight);
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    ((VideoRenderer *)context)->setSourceDimensions(maxWidth, maxHeight);
    return XSTX_RESULT_OK;
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Video Allocator C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
XStxResult allocatorInit(void *context, uint32_t maxWidth, uint32_t maxHeight)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((VideoModule *)context)->initAllocator(maxWidth, maxHeight);
}
XStxResult allocatorGetFrame(void *context, uint32_t width, uint32_t height,
                             XStxRawVideoFrame **reservedFrame)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((VideoModule *)context)->getFrame(width, height, reservedFrame);
}
XStxResult allocatorRecycleFrame(void *context,
                                 XStxRawVideoFrame *frameToRecycle)
{
    if (context == NULL)
    {
        assert(false);
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    return ((VideoModule *)context)->recycleFrame(frameToRecycle);
}


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Frame allocator
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
class RawVideoFrameAllocator :
    public mud::FixedSizePool<XStxRawVideoFrame>::Allocator
{

public:
    RawVideoFrameAllocator(uint32_t maxWidth, uint32_t maxHeight) :
        mMaxWidth(maxWidth),
        mMaxHeight(maxHeight)
    {
    }

    /**
     * Allocates XStxRawVideoFrame
     * @return XStxRawVideoFrame allocated frame
     */
    XStxRawVideoFrame* allocate()
    {

        // allocate memory
        XStxRawVideoFrame *frame = new XStxRawVideoFrame;

        memset(frame, 0, sizeof(XStxRawVideoFrame));

        // initialize newly allocated raw video frame
        frame->mWidth = mMaxWidth;
        frame->mHeight = mMaxHeight;

        frame->mPlanes[0] = 0;
        frame->mPlanes[1] = 0;
        frame->mPlanes[2] = 0;

        frame->mStrides[0] = 0;
        frame->mStrides[1] = 0;
        frame->mStrides[2] = 0;

        return frame;
    }

    /**
     * De-allocate XStxRawVideoFrame
     * @param[in] frame to be deallocated
     */
    void deallocate(XStxRawVideoFrame *frame)
    {
        delete frame;
    }

    // currently not used since we don't need to allocate data
    // AVFrame does that for us
    uint32_t mMaxHeight;
    uint32_t mMaxWidth;
};


//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          Video Module Code
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

/**
 *  Constructor
 */
VideoModule::VideoModule() :
    mDecoder(NULL), mRenderer(NULL),
    mPaused(false)
{
    memset(&mStxDecoder, 0, sizeof(mStxDecoder));
    memset(&mStxRenderer, 0, sizeof(mStxRenderer));
    memset(&mStxFrameAllocator, 0, sizeof(mStxFrameAllocator));
}

/**
 *  Destructor
 */
VideoModule::~VideoModule()
{
    delete mDecoder;

    // mRenderer is owned by our parent
}

/**
 * Initializes frame allocator
 * @param[in] maxWidth maximum width
 * @param[in] maxHeight maximum height
 */
XStxResult VideoModule::initAllocator(uint32_t maxWidth, uint32_t maxHeight)
{
    // instantiate allocator
    shared_ptr<RawVideoFrameAllocator>
        allocator(new RawVideoFrameAllocator(maxWidth, maxHeight));
    if (mFramePool.allocate(allocator, NUM_FRAMES_IN_POOL) != SIMPLE_RESULT_OK)
    {
        // failed allocate memory
        return XSTX_RESULT_OUT_OF_MEMORY;
    }
    return XSTX_RESULT_OK;
}

/**
 * Fetch a video frame if available
 * @param[in] width frame width. currently unused
 * @param[in] height frame height. currently unused
 * @param[out] reservedFrame to return pointer to video frame
 */
XStxResult VideoModule::getFrame(uint32_t width, uint32_t height,
                                 XStxRawVideoFrame **reservedFrame)
{
    if (mPaused)
    {
        return XSTX_RESULT_FRAME_NOT_AVAILABLE;
    }

    XStxRawVideoFrame *outFrame = NULL;
    // fetch a frame from frame pool
    if (mFramePool.getElement(outFrame) != SIMPLE_RESULT_OK)
    {
        // no frame is available
        return XSTX_RESULT_FRAME_NOT_AVAILABLE;
    }

    // successfully fetched a frame
    *reservedFrame = outFrame;
    return XSTX_RESULT_OK;
}

/**
 * Recycle video frame
 * @param[in] frameToRecycle pointer to video frame being recycled
 */
XStxResult VideoModule::recycleFrame(XStxRawVideoFrame *frameToRecycle)
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
 * Initialize video module
 * @param[in] clientHandle handle to XStx client
 * @param[in] rw renderer
 */
bool VideoModule::initialize(XStxClientHandle mClientHandle, VideoRenderer &vr)
{
    LOGI("VideoModule::initialize");

    // initialize frame allocator
    mStxFrameAllocator.mInitFcn = &allocatorInit;
    mStxFrameAllocator.mInitCtx = this;
    mStxFrameAllocator.mGetVideoFrameBufferFcn = &allocatorGetFrame;
    mStxFrameAllocator.mGetVideoFrameBufferCtx = this;
    mStxFrameAllocator.mRecycleVideoFrameBufferFcn = &allocatorRecycleFrame;
    mStxFrameAllocator.mRecycleVideoFrameBufferCtx = this;
    mStxFrameAllocator.mSize = sizeof(mStxFrameAllocator);
    if (XStxClientSetVideoFrameAllocator(mClientHandle, &mStxFrameAllocator)
        != XSTX_RESULT_OK)
    {
        LOGE("Failed to SetVideoFrameAllocator.\n");
        return false;
    }

    // initialize renderer
    mRenderer = &vr;

    mStxRenderer.mRenderVideoFrameFcn = &renderFrame;
    mStxRenderer.mRenderVideoFrameCtx = mRenderer;
    mStxRenderer.mSetMaxResolutionFcn = &rendererMaxResolution;
    mStxRenderer.mSetMaxResolutionCtx = mRenderer;
    mStxRenderer.mSize = sizeof(mStxRenderer);
    if (XStxClientSetVideoRenderer(mClientHandle, &mStxRenderer)
        != XSTX_RESULT_OK)
    {
        LOGE("Failed to set SetVideoRenderer.\n");
        return false;
    }

    // initialize decoder
    mDecoder = newVideoDecoder();
    if (!mDecoder)
    {
        return false;
    }

    mRenderer->setDecodeType(mDecoder->getDecodeType());

    mStxDecoder.mStartFcn = &videoDecoderStart;
    mStxDecoder.mStartCtx = mDecoder;
    mStxDecoder.mDecodeVideoFrameFcn = &decodeFrame;
    mStxDecoder.mDecodeVideoFrameCtx = mDecoder;
    mStxDecoder.mGetCapabilitiesCtx = this;
    mStxDecoder.mGetCapabilitiesFcn = &pipelineVideoDecoderGetCapabilities;
    mStxDecoder.mSize = sizeof(mStxDecoder);

    XStxResult result = XStxClientSetVideoDecoder(mClientHandle, &mStxDecoder);
    if (result != XSTX_RESULT_OK)
    {
        LOGV("Failed to set SetVideoDecoder:%d", result);
        return false;
    }

    // general client capability
    if (isChromaSamplingSupported(XSTX_CHROMA_SAMPLING_YUV420))
    {
        // YUV420 is supported
        if (XSTX_RESULT_OK == XStxClientAddChromaSamplingOption(
                mClientHandle, XSTX_CHROMA_SAMPLING_YUV420))
        {
            printf("Successfully notified of YUV420 capability\n");
        }
        else
        {
            printf("Failed to register YUV420 capability\n");
        }
    }
    else
    {
        printf("YUV420 is not supported...\n");
    }
    if (isChromaSamplingSupported(XSTX_CHROMA_SAMPLING_YUV444))
    {
        // YUV444 is supported
        if (XSTX_RESULT_OK == XStxClientAddChromaSamplingOption(
                mClientHandle, XSTX_CHROMA_SAMPLING_YUV444))
        {
            printf("Successfully notified of YUV444 capability\n");
        }
        else
        {
            printf("Failed to register YUV444 capability\n");
        }
    }
    else
    {
        printf("YUV444 is not supported...\n");
    }
    
    //We should always unpause when initialized (useful for clients that connect to multiple streams in one session)
    pause(false);
    
    return true;
}

bool VideoModule::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    return mDecoder->isChromaSamplingSupported(chromaSampling)
        && mRenderer->isChromaSamplingSupported(chromaSampling);
}

bool VideoModule::receivedClientConfiguration(const XStxClientConfiguration* config)
{
    // received client configuration
    // relay to video decoder / renderer
    return mDecoder->receivedClientConfiguration(config)
        && mRenderer->receivedClientConfiguration(config);
}

void VideoModule::stop()
{
    pause(true);

    // Tell the decoder to release its assets
    if (mDecoder)
    {
        mDecoder->release();
    }
}

void VideoModule::pause(bool pause)
{
    if (mPaused != pause)
    {
        LOGV("VideoModule Pause: %s", pause ? "true" : "false");
        mPaused = pause;
    }
}

void VideoModule::setKeyboardOffset(int offset)
{
    if (mRenderer)
    {
        mRenderer->setKeyboardOffset(offset);
    }
}
