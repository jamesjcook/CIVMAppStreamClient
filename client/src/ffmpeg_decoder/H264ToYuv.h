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


#ifndef _included_H264ToYuv_h
#define _included_H264ToYuv_h

#include "VideoDecoder.h"

extern "C"
{
#include "libavcodec/avcodec.h"
}

#include <map>


/**
 * An implementation of VideoDecoder that uses FFMPEG to do the
 * decoding.
 */
class H264ToYuv : public VideoDecoder
{

public:

    /** Constructor */
    H264ToYuv();

    /** Destructor */
    virtual ~H264ToYuv();

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

    /**
     * Pass general configuration parameters to renderer
     */
    bool receivedClientConfiguration(const XStxClientConfiguration* config);

    /**
     * Provide Chroma sampling capability
     */
    bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);

private:

    /**
     * Fetch A/V frame associated with given encoded video frame
     * @param[in] dec encoded video frame
     * @param[out] avFrame associated AV frame
     */
    XStxResult getAvFrame(XStxRawVideoFrame *dec, AVFrame *&avFrame);

    /** FFmpeg context for decoding video */

    AVCodecContext *mCodecContext;
    AVPacket mAvPacket;

    /** A/V frame pool */
    std::map<uint8_t *, AVFrame *> planeToAvFrame;

    XStxChromaSampling mChromaSampling;
};


#endif
