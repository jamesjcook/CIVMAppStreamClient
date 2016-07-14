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


#ifndef __AppStreamSampleClient__OSXVideoDecoder__
#define __AppStreamSampleClient__OSXVideoDecoder__

#include <VideoDecodeAcceleration/VDADecoder.h>

#include <map>

#include "VideoDecoder.h"

class OSXVideoDecoder : public VideoDecoder
{
    
    public:
    OSXVideoDecoder();
    
    /**
     * Initialize the decoder.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult init();
    
    /**
     * Decode a frame.
     *
     * @param[in] enc Frame to be decoded.
     * @param[out] dec Holder for decoded frame.
     */
    virtual XStxResult decodeFrame(XStxEncodedVideoFrame *enc, XStxRawVideoFrame *dec=NULL);
    
    /**
     * Return the decode type
     *
     * @return DECODE_PLANES
     */
    virtual EDecodeType getDecodeType()
    {
        return DECODE_PLANES;
    }
    
    /**
     * Callback for getting the decoded frame from the VDADecoder
     *
     * @param[in] frameInfo dictionary created when frame was passed to VDADecode
     * @param[in] status set by VDADecode
     * @param[in] infoFlags set by VDADecode
     * @param[in] imageBuffer CVImage buffer holding the decoded frame
     */
    void frameDecoded(CFDictionaryRef    frameInfo,
                      OSStatus           status,
                      uint32_t           infoFlags,
                      CVImageBufferRef   imageBuffer);

    /**
     * Pass general configuration parameters to renderer
     */
    bool receivedClientConfiguration(const XStxClientConfiguration* config);
    
    /**
     * Provide Chroma sampling capability
     */
    bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);

    
    /**
     * Flag to set in order to disable hardware decoding
     */
    bool disableHardwareDecode = false;
    
    
    private:
    OSStatus CreateDecoder(SInt32 inHeight, SInt32 inWidth,
                           OSType inSourceFormat, CFDataRef inAVCCData,
                           VDADecoder *decoderOut);
    
    /**
     * Converts the frame to the format needed for VDADecoder (4-byte size values
     * instead of 00 00 01 NAL headers) and returns it as a CFData
     *
     * @param[in] pointer to the beginning of the frame data
     * @param[in] size of the frame data passed in
     */
    CFDataRef convertFrameToData ( uint8_t *dataStart, uint32_t dataSize);

    VideoDecoder * mSWDelegate ;
    uint8_t * mInitFrame ;
    int mInitFrameLength;
    
    bool mFoundInitFrame ;
    
    VDADecoder mVDADecoder;
    
    VideoDecoder *ffmpegDecoder;
    
    bool mUseFFmpeg;
    
    pthread_mutex_t   mFrameDecodeMutex;
    
    uint32_t    videoWidth;
    uint32_t    videoHeight;
    
    XStxChromaSampling mChromaSampling;
};

#endif /* defined(__AppStreamSampleClient__OSXVideoDecoder__) */