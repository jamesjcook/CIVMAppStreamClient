/*
 * Copyright 2013 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *      http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */

#include "AndroidVideoDecoder.h"
#include "ffmpeg_decoder/H264ToYuv.h"
#include "platformBindings.h"
#include "mud/threading/ThreadUtil.h"
#include "AppStreamWrapper.h"

#undef LOG_TAG
#define LOG_TAG "AndroidVideoDecoder"
#include "log.h"

#include <assert.h>
#include <memory.h>

AndroidVideoDecoder::AndroidVideoDecoder() :
    mSWDelegate(NULL),
    mInitFrame(NULL),
    mFoundInitFrame(false),
    mClosing(false),
    mClosed(false)
{
}

AndroidVideoDecoder::~AndroidVideoDecoder()
{
    delete mSWDelegate;
    mSWDelegate = NULL;

    LOGV("AndroidVideoDecoder::~AndroidVideoDecoder()");
    delete[] mInitFrame;
    release();
    LOGV("AndroidVideoDecoder::~AndroidVideoDecoder() released/complete");
}


/*
 findBlockStart() looks for the first NAL unit header in the
 given block.

 A NAL unit starts with 0,0,1 or 0,0,0,1 and then a "type" byte.

 The first call to findBlockStart() should find the very first
 block; subsequent calls (to find the end of the block/start of the next
 block) are offset exactly enough so that "start" points at the type byte.

 In this way findBlockStart() groups all of the "config"-style blocks
 together into one big block, which mimics how Android MediaCodec seems to
 expect it.
*/

uint8_t* findBlockStart(uint8_t *start, uint8_t *end)
{
    // we're lumping together all the "config" frames as one big frame
    bool inConfig = (start[0] == 0x67 || (start[0] == 1 && start[1] == 0x67));

    while (end > start)
    {
        start = (uint8_t *)memchr(start, 0, end - start);

        // no more zeros? or no more room for a header + data? None to find.
        if (start == NULL || (end - start) <= 5)
        {
            return NULL;
        }
        else
        {
            if (start[1] != 0)
            {
                start++;
                continue;
            }

            // 0 0 1 is valid
            if (start[2] == 1)
            {
                if (inConfig && (start[3] == 0x68 || start[3] == 0x6))
                {
                    start++;
                    continue;
                }
                return start;
            }

            if (start[2] != 0)
            {
                start++;
                continue;
            }

            // 0 0 0 1 is valid
            if (start[3] == 1)
            {
                if (inConfig && (start[4] == 0x68 || start[4] == 0x6))
                {
                    start++;
                    continue;
                }
                return start;
            }

            start++;
        }

        return start;
    }

    return NULL;
}

/**
 * Decode a frame.
 *
 * @param[in] enc Frame to be decoded.
 * @param[out] dec Holder for decoded frame.
 */
XStxResult AndroidVideoDecoder::decodeFrame(XStxEncodedVideoFrame *enc, XStxRawVideoFrame *dec)
{
    if (mClosing)
    {
        mClosed = true;
        return XSTX_RESULT_INVALID_STATE;
    }

    if (androidHWDecodeAvailable())
    {
        // If we cached an init frame earlier (before the HW renderer was ready for it),
        // then send it now.
        if (mInitFrame)
        {
            int size = 0;
            uint8_t *buffer = (uint8_t *)androidGetHWBuffer(&size);

            if (!buffer)
            {
                // HW still not read; bail, but leave the init frame where it is.
                return XSTX_RESULT_VIDEO_DECODING_ERROR;
            }
            memcpy(buffer, mInitFrame, mInitFrameLength);
            androidReleaseHWBuffer(mInitFrameLength, 0);
            delete[] mInitFrame;
            mInitFrame = NULL;
        }

        uint8_t *blockEnd = enc->mData + enc->mDataSize;
        uint8_t *start = findBlockStart(enc->mData, blockEnd);

        if (!start)
        {
            uint8_t *buffer = enc->mData;
            LOGE("Header not found: Actual buffer header %d,%d,%d,%d,0x%x (0x%x)", buffer[0], buffer[1], buffer[2], buffer[3], buffer[4], buffer[5]);
            return XSTX_RESULT_VIDEO_DECODING_ERROR;
        }

        uint8_t *end = findBlockStart(start + 3, blockEnd); // skip over the first few bytes

        if (!end)
        {
            end = blockEnd;
        }

        // The exit case is tested below.
        while (true)
        {
            if (!mFoundInitFrame)
            {
                if (start[3] != 0x67 && start[4] != 0x67)
                {
                    LOGV("Skipping non-init frame 0x%x/0x%x", start[3], start[4]); // set it to the next valid start
                    start = end;
                    if (start == blockEnd)
                    {
                        break;
                    }
                    end = findBlockStart(end + 4, blockEnd);
                    if (!end)
                    {
                        end = blockEnd;
                    }
                    continue;
                }
                mFoundInitFrame = true;
            }

            int size = 0;
            uint8_t *buffer = (uint8_t *)androidGetHWBuffer(&size);
            if (!buffer)
            {
                if (start[3] == 0x67 || start[4] == 0x67)
                {
                    // cache our init frame!
                    mInitFrameLength = end - start;
                    mInitFrame = new uint8_t[mInitFrameLength];
                    LOGV("Cached init frame: %d bytes", mInitFrameLength);
                    memcpy(mInitFrame, start, mInitFrameLength);
                }
                LOGE("decodeFrame: No buffer; expected to see one!");
                break; //return XSTX_RESULT_INVALID_STATE ;
            }
            if (size < (end - start))
            {
                LOGE("decodeFrame: Buffer too small!");
                return XSTX_RESULT_INVALID_STATE;
            }

            int length = end - start;
            if (start[2] == 1)
            {
                buffer[0] = 0;
                memcpy(buffer + 1, start, end - start);
                ++length;
            }
            else
            {
                memcpy(buffer, start, end - start);
            }

            if (end >= blockEnd)
            {
                // if we've sent the last NAL unit, then wait until it's rendered
                androidReleaseHWBuffer(length, 30000);
                break; // and in this case we're done
            }
            else
            {
                androidReleaseHWBuffer(length, 0);
            }

            // set it to the next valid start
            start = end;
            end = findBlockStart(end + 4, blockEnd);
            if (!end)
            {
                end = blockEnd;
            }
        }

        static uint8_t dummy = 0;
        dec->mWidth = androidGetOutputBufferWidth();
        dec->mHeight = androidGetOutputBufferHeight();
        dec->mTimestampUs = enc->mTimestampUs;
        dec->mPlanes[0] = &dummy;
        dec->mPlanes[1] = &dummy;
        dec->mPlanes[2] = &dummy;
        dec->mStrides[0] = dec->mWidth;
        dec->mStrides[1] = dec->mWidth;
        dec->mStrides[2] = dec->mWidth;
        dec->mBufferSizes[0] = 1;
        dec->mBufferSizes[1] = 1;
        dec->mBufferSizes[2] = 1;

        return XSTX_RESULT_OK;
    }
    else // If hardware isn't active, then use FFmpeg
    {
        if (!mSWDelegate)
        {
            init();
        }
        return mSWDelegate->decodeFrame(enc, dec);
    }
}

/**
 * Initialize the decoder.
 *
 * @return XSTX_RESULT_OK on success.
 */
XStxResult AndroidVideoDecoder::init()
{
    if (androidHWDecodeAvailable())
    {
        return XSTX_RESULT_OK;
    }

    mSWDelegate = new(std::nothrow) H264ToYuv();
    return mSWDelegate->init();
}

extern AppStreamWrapper *gAppStreamWrapper;
void AndroidVideoDecoder::release()
{
    if (mClosed) return;

    mClosing = true;

    gAppStreamWrapper->pause(false);

    int timeout = 10; // 50ms * 10 = 0.5 seconds

    while ((!mClosed) && (--timeout))
    {
        mud::ThreadUtil::sleep(50);
    }

    if (mClosed)
    {
        // If we make it here, then go ahead and delete the
        // delegate. If we don't, then leave it around and
        // it will be deleted later -- or reused.
        delete mSWDelegate;
        mSWDelegate = NULL;
    }
}

/**
 * Provide Chroma sampling capability.
 */
bool AndroidVideoDecoder::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    // If we have hardware, then ask the hardware whether we have XSTX_H264_PROFILE_HIGH444.
    // If not, then we fall back to 420.
    if (androidHWDecodeAvailable() && (androidGetProfile()!=XSTX_H264_PROFILE_HIGH444))
    {
        return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420;
    }
    else
    {
        return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420
            || chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
    }
}
