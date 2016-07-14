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


#ifndef _included_HeadlessVideoDecoder_h
#define _included_HeadlessVideoDecoder_h

#include "VideoDecoder.h"

#include <map>


/**
 * An implementation of VideoDecoder that uses FFMPEG to do the
 * decoding.
 */
class HeadlessVideoDecoder : public VideoDecoder
{

public:

    /** Constructor */
    HeadlessVideoDecoder();

    /** Destructor */
    virtual ~HeadlessVideoDecoder();

    /**
     * Initialize the FFmpeg decoder.
     *
     * @return XSTX_RESULT_OK on success; otherwise an appropriate error code.
     */
    virtual XStxResult init();

    /**
     * Decode frame
     * @param[in] enc frame to be decoded
     * @param[out] dec holder for decoded frame
     */
    XStxResult decodeFrame(
        XStxEncodedVideoFrame *enc,
        XStxRawVideoFrame *dec);
};


#endif
