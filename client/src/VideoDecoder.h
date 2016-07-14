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


#ifndef _included_VideoDecoder_h
#define _included_VideoDecoder_h

#include <stddef.h>

#include "XStx/common/XStxAPI.h"

/**
 * The abstract video decoder.
 */
class VideoDecoder
{
public :
    /**
     * Possible decoder types.
     */
    enum EDecodeType
    {
        /**
         * Decode type is undefined/uninitialized.
         */
        DECODE_UNDEFINED,

        /**
         * Decode is done as three distinct planes of Y, U, and V, and
         * the width of the U and V planes equals the stride. The U and V planes
         * are each half the actual size of the Y plane in each dimension.
         */
        DECODE_PLANES,

        /**
         * Decode is done as three distinct planes of Y, U, and V, and
         * the width of the U and V planes equals the stride. The planes are all
         * the same size.
         */
        DECODE_PLANES444,

        /**
         * Decode is done as a Y plane and an interleaved U and V plane.
         * The width of the U and V planes equals half the stride.
         */
        DECODE_INTERLEAVED,

        /**
         * Decode is done directly to a PBuffer, which then needs to be rendered
         * as RGB to the screen, using the proper PBuffer extension.
         */
        DECODE_PBUFFER,
    };
    /**
     * Destructor.
     */
    virtual ~VideoDecoder() { };

    /**
     * Initialize the decoder.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult init() = 0;

    /**
     * Return the decode type (current planes and interleaved).
     *
     * @return DECODE_PLANES if the U and V planes are distinct memory blocks
     *     (i.e., the U and V stride == the plane width). Returns DECODE_INTERLEAVED
     *     if the U and V planes are interleaved by row (i.e., the U and V
     *     stride == 2 x the plane width).
     */
    virtual EDecodeType getDecodeType()
    {
        return DECODE_PLANES;
    }

    /**
     * Decode a frame.
     *
     * @param[in] enc Frame to be decoded.
     * @param[out] dec Holder for decoded frame.
     */
    virtual XStxResult decodeFrame(XStxEncodedVideoFrame *enc, XStxRawVideoFrame *dec = NULL) = 0;

    /**
     * Release the decoder resources and thread bindings. Unused on most
     * platforms, but on Android this is needed to properly release the
     * decoder thread.
     */
    virtual void release() { }

    /**
     * Pass general configuration parameters to decoder
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool receivedClientConfiguration(const XStxClientConfiguration* config) { return true; }

    /**
     * Provide Chroma sampling capability.
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool isChromaSamplingSupported(XStxChromaSampling chromaSampling) { return false; }
};

#endif // _included_VideoDecoder_h

