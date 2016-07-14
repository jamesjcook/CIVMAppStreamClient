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

#ifndef _included_AndroidVideoDecoder_h
#define _included_AndroidVideoDecoder_h

#include "VideoDecoder.h"
#include "jniBindings.h"

class AndroidVideoDecoder : public VideoDecoder
{

public:
    /**
     *
     * Constructor.
     */
    AndroidVideoDecoder();

    /**
     *
     * Destructor.
     */
    virtual ~AndroidVideoDecoder();

    /**
     *
     * Initialize the decoder.
     *
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult init();

    /**
     *
     * Decode a frame.
     *
     *
     * @param[in] enc Frame to be decoded.
     * @param[out] dec Holder for decoded frame.
     */
    virtual XStxResult decodeFrame(XStxEncodedVideoFrame *enc, XStxRawVideoFrame *dec = NULL);

    /**
     *
     * Return the decode type (current planes and interleaved).
     *
     *
     * @return DECODE_PLANES
     */
    virtual EDecodeType getDecodeType()
    {
        if (androidHWDecodeAvailable())
        {
            return DECODE_PBUFFER;
        }
        else
        {
            return DECODE_PLANES;
        }
    }

    virtual void release();

    /**
     * Provide Chroma sampling capability.
     */
    virtual bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);

private:
    VideoDecoder *mSWDelegate;
    uint8_t *mInitFrame;
    int mInitFrameLength;

    bool mFoundInitFrame;
    bool mClosing;
    volatile bool mClosed;
};

#endif // _included_AndroidVideoDecoder_h
