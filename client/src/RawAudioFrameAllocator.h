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


#ifndef _include_RawAudioFrameAllocator_h
#define _include_RawAudioFrameAllocator_h

/**
 * Allocator Helper
 */
class RawAudioFrameAllocator :
    public mud::FixedSizePool<XStxRawAudioFrame>::Allocator
{

public:
    /**
     * Constructor.
     *
     * @param maxSize The largest size of audio frame we need to accept.
     */
    RawAudioFrameAllocator(uint32_t maxSize) :
        mMaxSize(maxSize)
    {
    }

    /**
     * Allocates XStxRawAudioFrame
     * - instantiates XStxRawAudioFrame with trailing buffer
     * @return XStxRawAudioFrame allocated frame
     */
    XStxRawAudioFrame* allocate()
    {
        // allocate memory
        uint8_t *buffer = new uint8_t[mMaxSize + sizeof(XStxRawAudioFrame)];
        if (!buffer)
        {
            return NULL;
        }
        // populate XStxRawAudioFrame part
        XStxRawAudioFrame *frame = (XStxRawAudioFrame *)buffer;
        memset(frame, 0, sizeof(XStxRawAudioFrame));
        frame->mDataSize = 0;
        frame->mBufferSize = mMaxSize;
        frame->mData = buffer + sizeof(XStxRawAudioFrame);
        frame->mTimestampUs = 0;
        frame->mSize = sizeof(XStxRawAudioFrame);

        // successfully allocated a frame
        return frame;
    }

    /**
     * De-allocate XStxRawAudioFrame
     * - agnostic to underlying structure
     * @param[in] frame to be deallocated
     */
    void deallocate(XStxRawAudioFrame *frame)
    {
        uint8_t *buffer = (uint8_t *)frame;
        delete[] buffer;
    }

    /**
     * The audio frame size.
     */
    uint32_t mMaxSize;
};

#endif // _include_RawAudioFrameAllocator_h
