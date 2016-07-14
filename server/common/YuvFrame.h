/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#ifndef YUVFRAME_H_
#define YUVFRAME_H_

#include <stdint.h>
#include <stdio.h>

#include "XStx/common/XStxAPI.h"

/**
 * Encapsulates a buffer and metadata for a YUV frame. Provides
 * region-of-interest based retrieval of a sub-frame. For instance,
 * the YuvFrame can be intialized to point to buffers that contain the
 * planes of a 1080p YUV frame, while the region of interest is then
 * set to a 720p frame (or other resolution of your choice) within
 * that 1080p frame.
 *
 * By default the region of interest is the entire YUV frame.
 */
class YuvFrame : public XStxRawVideoFrame
{

public:
    YuvFrame();
    ~YuvFrame();

    /**
     * Deallocates the sample buffer. ~YuvFrame() calls this.
     */
    void freePlanes();

    /**
     * Allocates the Y, U, and V planes, with 4:2:0 sizes.  Y is width x
     * height. U and V are (width/2) x (height/2).
     *
     * The region of interest is set to the entire YUV frame.
     *
     * @param[in] width the width of the Y-plane in pixels.
     * @param[in] height the height of the Y-plane in pixels.
     * @return true if allocation succeeds, false otherwise.
     */
    bool init(uint32_t width, uint32_t height);

    /**
     * Same as init(uint32_t width, uint32_t height) but you specify a filename
     * where the raw yuv data should be loaded from.
     *
     * The region of interest is the entire YUV frame.
     *
     * @param[in] yuvFilename: filename
     *            where the raw yuv data should be loaded from.
     * @param[in] width the width of the Y-plane in pixels.
     * @param[in] height the height of the Y-plane in pixels.
     * @return true if allocation and reading from file succeeds,
     *         false otherwise.
     */
    bool init(const char* yuvFilename, uint32_t width, uint32_t height);

    /**
     * @return a length-3 array of plane pointers to the region of
     * interest frame within the allocated YUV frame.
     */
    uint8_t** getPlanes() {return pRoiPlanes;}

    /**
     * @return a length-3 array of strides, for Y,U,and V.
     */
    int32_t*  getStrides() {return strides;}

    /**
     * @return the width of the region of interest frame inside the
     * allocated YUV frame.
     */
    uint32_t getWidth() {return roiWidth;}

    /**
     * @return the height of the region of interest frame insided the
     * allocated YUV frame.
     */
    uint32_t getHeight() {return roiHeight;}

    /** Helper function to remember index used in recycling */
    int getFramePoolIndex() { return mFramePoolIndex; }

    /** Helper function to set index used in recycling */
    void setFramePoolIndex(const int index) { mFramePoolIndex = index; }

    /**
     * Sets the region of interest frame inside the YUV frame. After
     * successful return from this method, getPlanes(), getWidth(), and
     * getHeight() will return the values associated with the region of
     * interest sub-frame.
     *
     * @param[in] roiWidth the region of interest frame width.  Must be
     *            less than or equal to the allocated frame width.
     * @param[in] roiHeight the region of interest frame height.  Must be
     *            less than or equal to the allocated frame height.
     * @param[in] roiX the offset of the region of interest from the left
     *            of the allocated frame. roiX + roiWidth must be less than
     *            the width of the allocated frame.
     * @param[in] roiY the offset of the region of interest from the top
     *            of the allocated frame. roiY + roiHeight must be less than
     *            the height of the allocated frame.
     * @return true on success, false if the roiX + roiWidth is greater
     *         than the width of the allocated frame, or if roiY +
     *         roiHeight is greater than the height of the allocated
     *         frame.
     */
    bool setRegionOfInterest(uint32_t roiWidth, uint32_t roiHeight,
                   uint32_t roiX, uint32_t roiY);

    /**
     * Copies region of interest parameters from the frame.  The frame
     * must have the same allocated frame size.
     * @param[in]  frame the frame whose region of interest parameters are
     *             copied.
     * @ return true on success, false if the frame has different allocated size.
     */
    bool copyRegionOfInterestParams(const YuvFrame* frame);

    /**
     * Dumps the allocated YUV planes to a file.
     * @param[in] fp, an open file pointer.
     * @param[out] numBytesWritten the number of byte written to the file.
     * @return true on success, false otherwise.
     */
    bool writeToFile(FILE* fp, uint32_t& numBytesWritten);

    /**
     * Dumps the regioin of interest YUV planes to a file.
     * @param[in] fp, an open file pointer.
     * @param[out] numBytesWritten the number of byte written to the file.
     * @return true on success, false otherwise.
     */
    bool writeRoiToFile(FILE* fp, uint32_t& numBytesWritten);

    /**
     * Loads a YUV from a file to allocated planes for an initialized
     * XYuvFrame.
     * @param[in] filename the file containing the YUV420 frame that is the same
     *            width and height that the XYuvFrame has been initialized to.
     * @return true on success, false if the file can't be found or the XYuvFrame
     *         hasn't been initialized.
     */
    bool readFromFile(const char* filename);

private:
    bool writeToFile(FILE* fp, uint32_t& numBytesWritten,
        uint32_t aWidth, uint32_t aHeight, uint8_t** aPlanes);

private:
    // width of entire frame
    uint32_t width;
    // height of entire frame
    uint32_t height;
    // video buffer
    uint8_t* pPlanes[3];
    // strides to be used in offset calculation
    int32_t  strides[3];
    // width of region-of-interest frame
    uint32_t roiWidth;
    // height of regioin-of-interest frame
    uint32_t roiHeight;
    // x-coordinate of upper-left corner of region-of-interest 
    uint32_t roiX;
    // y-coordinate of upper-left corner of region-of-interest
    uint32_t roiY;
    // buffer for region-of-interest
    uint8_t* pRoiPlanes[3];
    // field used for convenience when being recycled
    int mFramePoolIndex;
};
#endif /* YUVFRAME_H_ */
