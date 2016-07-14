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

#define _CRT_SECURE_NO_DEPRECATE
#include "YuvFrame.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

using namespace std;

YuvFrame::YuvFrame()
    : width(0)
    , height(0)
    , roiWidth(0)
    , roiHeight(0)
    , roiX(0)
    , roiY(0)
    , mFramePoolIndex(-1)
{
    for (int i = 0; i < 3; i++)
    {
        pPlanes[i] = 0;
        pRoiPlanes[i] = 0;
    }
    // XStxRawVideoFrame members
    mWidth = 0;
    mHeight = 0;
    mTimestampUs = 0;
    mSize = sizeof(XStxRawVideoFrame);
}

YuvFrame::~YuvFrame()
{
    freePlanes();
}

/**
 * Deallocates the sample buffer. ~YuvFrame() calls this.
 */
void YuvFrame::freePlanes()
{
    for (int i = 0; i < 3; i++) {
        if (pPlanes[i] != NULL) {
            delete [] pPlanes[i];
        }
        pPlanes[i] = NULL;
    }
    // NOTE: pRoiPlanes point to the buffers allocated to pPlanes
    // and should not be deleted.
}

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
bool YuvFrame::init(const char* yuvFilename, uint32_t awidth, uint32_t aheight)
{
    if ( !init(awidth, aheight) )
    {
        return false;
    }
    if ( !(readFromFile(yuvFilename)) )
    {
        return false;
    }
    return true;
}

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
bool YuvFrame::init(uint32_t awidth, uint32_t aheight)
{
    width = awidth;
    height= aheight;

    freePlanes();

    if ( width % 2 != 0 ||  height % 2 != 0 )
    {
        cout << "Error: height and width must be divisible by 2" << endl;
        return false;
    }

    pPlanes[0] = new uint8_t[ width * height ];
    pPlanes[1] = new uint8_t[ width * height / 4 ];
    pPlanes[2] = new uint8_t[ width * height / 4 ];

    if( NULL == pPlanes[0] || NULL == pPlanes[1] || NULL == pPlanes[2])
    {
        freePlanes();
        return false;
    }

    strides[0] = width;
    strides[1] = width/2;
    strides[2] = width/2;

    for ( int i = 0; i < 3; i++ )
    {
        pRoiPlanes[i] = pPlanes[i];
    }
    roiWidth = width;
    roiHeight = height;
    roiX = 0;
    roiY = 0;

    return true;
}

bool YuvFrame::writeToFile(
    FILE* fp, uint32_t& numBytesWritten,
    uint32_t aWidth, uint32_t aHeight, uint8_t** aPlanes)
{
    numBytesWritten = 0;
    uint32_t heights[3] = { aHeight, aHeight/2, aHeight/2 };
    uint32_t widths[3] = { aWidth, aWidth/2, aWidth/2 };
    uint8_t* planes[3] = { aPlanes[0], aPlanes[1], aPlanes[2] };
    for( int p = 0; p < 3; p++)
    {
        for( uint32_t i = 0; i < heights[p]; i++)
        {
            if ( fwrite(planes[p] + i * strides[p], widths[p], 1, fp) != 1 )
            {
                return false;
            } else {
                numBytesWritten += widths[p];
            }
        }
    }
    return true;
}

/**
 * Dumps the allocated YUV planes to a file.
 * @param[in] fp, an open file pointer.
 * @param[out] numBytesWritten the number of byte written to the file.
 * @return true on success, false otherwise.
 */
bool YuvFrame::writeToFile(FILE* fp, uint32_t& numBytesWritten)
{
    return writeToFile(fp, numBytesWritten, width, height, pPlanes);
}

/**
 * Dumps the regioin of interest YUV planes to a file.
 * @param[in] fp, an open file pointer.
 * @param[out] numBytesWritten the number of byte written to the file.
 * @return true on success, false otherwise.
 */
bool YuvFrame::writeRoiToFile(FILE* fp, uint32_t& numBytesWritten)
{
    return writeToFile(fp, numBytesWritten, roiWidth, roiHeight, pRoiPlanes);
}

/**
 * Loads a YUV from a file to allocated planes for an initialized
 * XYuvFrame.
 * @param[in] filename the file containing the YUV420 frame that is the same
 *            width and height that the XYuvFrame has been initialized to.
 * @return true on success, false if the file can't be found or the XYuvFrame
 *         hasn't been initialized.
 */
bool YuvFrame::readFromFile(const char* name)
{
    FILE* fp = fopen(name, "rb");
    if ( NULL == fp )
    {
        cout << "Error: YuvFrame::readFromFile couldn't open " << name << endl;
        return false;
    }
    if ( NULL == pPlanes[0] || NULL == pPlanes[1] || NULL == pPlanes[2] )
    {
        cout << "Error: YuvFrame::readFromFile: YuvFrame not initialized."<< endl;
        return false;
    }

    uint32_t heights[3] = { height, height/2, height/2 };
    uint32_t widths[3] = { width, width/2, width/2 };
    uint8_t* planes[3] = { pPlanes[0], pPlanes[1], pPlanes[2] };
    for ( int p = 0; p < 3; p++ )
    {
        for ( uint32_t i = 0; i < heights[p]; i++ )
        {
            if ( fread(planes[p]+i*strides[p], widths[p], 1, fp) != 1 )
            {
                cout << "Error: YuvFrame::readFromFile() couldn't read plane: "
                     << p << " line: " << i << endl;
                fclose(fp);
                return false;
            }
        }
    }
    fclose(fp);
    return true;
}

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
bool YuvFrame::setRegionOfInterest(
    uint32_t aWidth, uint32_t aHeight, uint32_t aX, uint32_t aY)
{
    if ( aX + aWidth > width || aY + aHeight > height )
    {
        cout << "Error: YuvFrame::setRegionOfInterest: ROI frame is extends "
            "beyond the borders of the allocated frame. Allocated width: "
             << width << " . Allocated height: " << height
             << ". ROI x: " << aX << ". ROI y: " << aY
             << ". ROI width: " << aWidth << ". ROI height: " << aHeight
             << endl;
        return false;
    }
    roiX = aX;
    roiY = aY;
    roiWidth = aWidth;
    roiHeight = aHeight;
    pRoiPlanes[0] = pPlanes[0] + (aY * strides[0]) + aX;
    pRoiPlanes[1] = pPlanes[1] + ((aY/2) * strides[1]) + (aX/2);
    pRoiPlanes[2] = pPlanes[2] + ((aY/2) * strides[2]) + (aX/2);
    return true;
}

/**
 * Copies region of interest parameters from the frame.  The frame
 * must have the same allocated frame size.
 * @param[in]  frame the frame whose region of interest parameters are
 *             copied.
 * @ return true on success, false if the frame has different allocated size.
 */
bool YuvFrame::copyRegionOfInterestParams(const YuvFrame* frame)
{
    if ( frame->height != height || frame->width != width )
    {
        cout << "Error: YuvFrame::copyRegionOfInterestParams: "
            "Frames have different dimensions." << endl;
        return false;
    }
    roiX = frame->roiX;
    roiY = frame->roiY;
    roiWidth = frame->roiWidth;
    roiHeight = frame->roiHeight;
    pRoiPlanes[0] = pPlanes[0] + (roiY * strides[0]) + roiX;
    pRoiPlanes[1] = pPlanes[1] + ((roiY/2) * strides[1]) + (roiX/2);
    pRoiPlanes[2] = pPlanes[2] + ((roiY/2) * strides[2]) + (roiX/2);
    return true;
}

