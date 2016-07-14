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


#ifndef _included_VideoRenderer_h
#define _included_VideoRenderer_h

#include <stdint.h>
#include "XStx/client/XStxClientAPI.h"
#include "MUD/threading/WaitableLock.h"

#include "VideoDecoder.h"

/**
 * The base class of the video renderer. Handles queuing of frames
 * for the actual render, but the render itself happens (typically)
 * in another thread.
 */
class VideoRenderer
{
public:
    /**
     * Constructor.
     */
    VideoRenderer() :
        mDecodeType(VideoDecoder::DECODE_UNDEFINED),
        mScaled(false),
        mDisplayWidth(0),
        mDisplayHeight(0),
        mWidth(0),
        mHeight(0),
        mWidthOffset(0),
        mHeightOffset(0),
        mSourceWidth(0),
        mSourceHeight(0),
        mScale(0),
        mExiting(false),
        mFrameValid(false),
        mFrame(NULL)
    { };

    /**
     * Destructor.
     */
    virtual ~VideoRenderer();

    /**
     * Initialize the renderer.
     *
     * @return True on success.
     */
    virtual bool init() = 0;

    /**
     * Optional call to request a frame from the platform. On systems
     * that don't automatically render 30FPS or 60FPS, used to request
     * a new frame.
     */
    virtual void requestFrame() { };

    /**
     * Draw to the screen.
     *
     * The implementation of this function should call checkQueue()
     * to actuall render the frame.
     *
     * @return 1 if a new frame was drawn; 0 if not.
     */
    virtual int draw() = 0;

    /**
     * Called after all drawing is complete to present the frame.
     *
     * Optional call in some architectures (ones with an implicit "present"
     * or "swap").
     */
    virtual void post() { };

    /**
     * Return true if initialized. Implemenation should set mInitialized
     * to true after init() is called.
     */
    virtual bool isInitialized() { return (mWidth != 0) && (mHeight != 0); }

    /**
     * Queue the given frame to render. Blocks until the frame
     * has been queued so that, on return from this function, the
     * frame can be freed and reused.
     *
     * @param[in] frame to render
     */
    XStxResult postFrame(const XStxRawVideoFrame *frame);

    /**
     * Change width and height of the video source.
     *
     * @param[in] w width
     * @param[in] h height
     */
    void setSourceDimensions(uint32_t w, uint32_t h);

    /**
     * Change width and height of the video source.
     *
     * @param[in] w width
     * @param[in] h height
     * @param[in] forceRescale Force the view to be rescaled (useful for
     *       changing the window size).
     */
    virtual void setDisplayDimensions(uint32_t w, uint32_t h, bool forceRescale=true);

    /**
     * Set the video decode type.
     *
     * @see VideoDecoder::EDecodeType
     *
     * @param[in] decodeType New video decode type.
     */
    virtual void setDecodeType(VideoDecoder::EDecodeType decodeType)
    {
        mDecodeType = decodeType;
    }

    /**
     * Set the viewport. This is used to create the "black bars" required
     * when the aspect ratio of the source image doesn't match the aspect
     * ratio of the current display.
     *
     * @param[in] x      x offset
     * @param[in] y      y offset
     * @param[in] w      width
     * @param[in] h      height
     */
    virtual void setViewport(int x, int y, int w, int h)
    {
        mWidthOffset = x;
        mHeightOffset = y;
        mWidth = w;
        mHeight = h;
        setScaleAndOffset();
    }

    /**
     * A call that lets us know what offset we should use for the surface when
     * the keyboard is present.
     *
     * @param[in] offset Number of pixels to offset the view.
     */
    virtual void setKeyboardOffset(int offset)
    {
        (void)offset;
    }

    /**
     * Pause the video rendering. In the default implementation, does
     * nothing, but some implemenations need to know when to pause the
     * video.
     *
     * @param[in] pause  True to pause.
     */
    virtual void pause(bool pause)
    {
        // Nothing here, but some implementations need to know when
        // the app is paused.
    }

    /**
     * Get the width of the part of the display surface being used to display
     * the video stream. May be smaller than the entire display surface
     * because of black bars.
     *
     * @return Video surface width in display pixels.
     */
    int getWidth()
    {
        return mWidth;
    }

    /**
     * Get the height of the display surface being used to display
     * the video stream. May be smaller than the entire display surface
     * because of black bars.
     *
     * @return Video surface height in display pixels.
     */
    int getHeight()
    {
        return mHeight;
    }

    /**
     * Get the scale value that you need to multiply by incoming mouse or
     * touch coordinates to match the original screen coordinate system.
     *
     * @param[in] scale  Scale to multiply by incoming coordinates.
     * @param[in] x      Offset to add to x coordinates (before
     *               scaling).
     * @param[in] y      Offset to add to y coordinates (before
     *               scaling).
     */
    virtual void getScaleAndOffset(float &scale, int &x, int &y)
    {
        scale = mScale;
        x = -(int)mWidthOffset;
        y = -(int)mHeightOffset;
    }

    /**
     * Clear the screen; should behave gracefully even before the renderer has been
     * fully initialized.
     */
    virtual void clearScreen()
    {

    }

    /**
     * Pass general configuration parameters to decoder
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool receivedClientConfiguration(const XStxClientConfiguration* config) { return true; }

    /**
     * Provide Chroma sampling capability
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool isChromaSamplingSupported(XStxChromaSampling chromaSampling) { return false; }

    /**
     * Stop checking queue for new frames to render
     */
    void stop()
    {
        mExiting = true;
        mSampleLock.lock();
        mFrame = NULL;
        mSampleLock.signal();
        mSampleLock.unlock();
    }

protected:
    /**
     * Set the scale and offset values, calculated based on the relative
     * size of the source video and the display dimensions.
     */
    void setScaleAndOffset();

    /**
     * The current requested decode type.
     */
    VideoDecoder::EDecodeType mDecodeType;

    bool mScaled;               ///< Display has been properly scaled.
    uint32_t mDisplayWidth;     ///< Display width in pixels.
    uint32_t mDisplayHeight;    ///< Display height in pixels.
    uint32_t mWidth;            ///< Width of destination video rectangle in pixels.
    uint32_t mHeight;           ///< Height of destination video rectangle in pixels.
    uint32_t mWidthOffset;      ///< Offset of destination video rectangle in pixels.
    uint32_t mHeightOffset;     ///< Offset of destination video rectangle in pixels.
    uint32_t mSourceWidth;      ///< Width of source video in pixels.
    uint32_t mSourceHeight;     ///< Height of source video in pixels.

    float mScale;               ///< Amount to scale source video.

    bool mExiting;             ///< True if the app is exiting.

    /**
     * Do the work of rendering the incoming frame to a texture.
     */
    virtual void render() = 0;

    /**
     * Check to see if the queue has a frame in it, and if so, render
     * it. Should be called at the start of the user-defined draw()
     * function.
     *
     * @return 1 if the frame was rendered; 0 otherwise.
     */
    int checkQueue();


    /**
     * Have we ever rendered a valid frame?
     */
    bool mFrameValid;

    /**
     * Lock used to synchronize the transfer of a video frame to the
     * video rendering thread.
     */
    mud::WaitableLock mSampleLock;

    /**
     * The current video frame.
     */
    const XStxRawVideoFrame *mFrame;
};

#endif //_included_VideoRenderer_h

