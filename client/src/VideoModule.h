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


#ifndef _included_VideoModule_h
#define _included_VideoModule_h

#include "XStx/common/XStxAPI.h"
#include "XStx/client/XStxClientAPI.h"

#include "MUD/memory/FixedSizePool.h"

#include "VideoRenderer.h"
#include "VideoDecoder.h"

/**
 * VideoModule provides video frames and holds a VideoRenderer and a
 * VideoDecoder.
 *
 * Actual video decoder and video render used by VideoModule are declared
 * in a file (typically) called VideoPipeline.cpp. That file is defined
 * for each supported platform, and instantiates the appropriate video
 * decoder and renderer for its associated platform.
 */
class VideoModule
{
public:

    /**
     *  Constructor
     */
    VideoModule();

    /**
     *  Destructor
     */
    ~VideoModule();

    /**
     * Initialize video module
     * @param[in] clientHandle handle to XStx client
     * @param[in] vr VideoRenderer
     */
    bool initialize(XStxClientHandle mClientHandle, VideoRenderer &vr);

    /**
     * Initializes frame allocator
     * @param[in] maxWidth maximum width
     * @param[in] maxHeight maximum height
     */
    XStxResult initAllocator(uint32_t maxWidth, uint32_t maxHeight);

    /**
     * Fetch a video frame if available
     * @param[in] width frame width. currently unused
     * @param[in] height frame height. currently unused
     * @param[out] reservedFrame to return pointer to video frame
     */
    XStxResult getFrame(uint32_t width, uint32_t height,
                        XStxRawVideoFrame **reservedFrame);

    /**
     * Recycle video frame
     * @param[in] frameToRecycle pointer to video frame being recycled
     */
    XStxResult recycleFrame(XStxRawVideoFrame *frameToRecycle);

    /**
     * The app has gone to sleep; pause all processing.
     *
     * @param[in] pause  True to enter a paused state; false otherwise.
     */
    void pause(bool pause);

    /**
     * The app is exiting.
     */
    void stop();

    /**
     * Get the current video renderer.
     *
     * @return A pointer to the video renderer, or NULL if none has been created
     *     yet.
     */
    VideoRenderer* getRenderer() { return mRenderer; }

    /**
     * A call that lets us know what offset we should use for the surface when
     * the keyboard is present.
     *
     * @param[in] offset Number of pixels to offset the view.
     */
    void setKeyboardOffset(int offset);
    void showKeyboard(bool show);

    /**
     * Takes general configuration such as YUV444 support
     *
     * @return true if successful
     */
    bool receivedClientConfiguration(const XStxClientConfiguration* config);

private:

    bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);

    /**
     *  decoder
     */
    VideoDecoder *mDecoder;

    /**
     *  renderer
     */
    VideoRenderer *mRenderer;

    /**
     *  pool for video frames
     */
    mud::FixedSizePool<XStxRawVideoFrame> mFramePool;

    /**
     *  size of video frame pool
     */
    static const uint32_t NUM_FRAMES_IN_POOL = 7;

    XStxIVideoDecoder mStxDecoder;
    XStxIVideoRenderer mStxRenderer;
    XStxIRawVideoFrameAllocator mStxFrameAllocator;

    volatile bool mPaused;
};


#endif //_included_VideoModule_h
