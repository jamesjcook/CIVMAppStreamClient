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

#ifndef FRAMEALLOCATORS_H_
#define FRAMEALLOCATORS_H_

#include <string>
#include <assert.h>

#include "MUD/memory/FixedSizePool.h"

#include "XStx/common/XStxUtil.h"
#include "XStx/common/XStxResultAPI.h"
#include "YuvFrame.h"

/**
 * Allocator for audio frame pool
 * - allocates audio frame buffer with fixed buffer size
 */
class XStxRawAudioFrameAllocatorForPool :
    public mud::FixedSizePool< XStxRawAudioFrame >::Allocator {

public:

    /**
     * Constructor
     * @param[in] sizeInBytes size of audio frame buffer
     */
    XStxRawAudioFrameAllocatorForPool(const uint32_t sizeInBytes)
        : mSizeInBytes(sizeInBytes)
    {}

    // allocate audio frame buffer
    XStxRawAudioFrame* allocate()
    {
        XStxRawAudioFrame* frame = new XStxRawAudioFrame;
        frame->mData = new uint8_t[mSizeInBytes];
        frame->mBufferSize = mSizeInBytes;
        frame->mSize = sizeof(XStxRawAudioFrame);
        frame->mDataSize = 0;  // no data is filled yet
        return frame;
    }

    // free audio frame buffer
    void deallocate(XStxRawAudioFrame* frame)
    {
        if (NULL != frame)
        {
            delete [] frame->mData;
            delete frame;
            frame = NULL;
        }
    }

private:
    uint32_t const mSizeInBytes;
};

/**
 * Allocator for video frame pool
 * - allocates video frame buffer with fixed width and height
 * - reads original images from files
 */
class XStxYuvFrameAllocatorForPool :
    public mud::FixedSizePool< YuvFrame >::Allocator {

public:

    /**
     * Constructor
     * @param[in] yuvFilename filename for original image
     * @param[in] width width of image
     * @param[in] height height of image
     */
    XStxYuvFrameAllocatorForPool(
        std::string yuvFilename, int width, int height)
        : mWidth(width), mHeight(height), mYuvFilename(yuvFilename)
    {}

    YuvFrame* allocate()
    {
        return allocate(1);
    }

    /**
     * Reads from a file and allocate video frame
     * @return YuvFrame is successful, NULL otherwise
     * @param[in] index used to index filenames
     */
    YuvFrame* allocate(const int index)
    {
        YuvFrame* frame = new YuvFrame();
        if (!frame)
        {
            return NULL;
        }
        char name[50];
        snprintf(name, 50, mYuvFilename.c_str(), index);
        // initialize video frame
        if (!frame->init(name, mWidth, mHeight)) {
            delete frame;
            frame = NULL;
        }
        return frame;
    }

    /**
     * Deallocated video frame
     * @param[in] frame video frame to be de-allocated
     */
    void deallocate(YuvFrame* frame)
    {
        if (NULL != frame)
        {
            delete frame;
            frame = NULL;
        }
    }

private:
    const int mWidth;
    const int mHeight;
    std::string mYuvFilename;
};

#endif /* FRAMEALLOCATORS_H_ */
