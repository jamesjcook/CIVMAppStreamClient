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


#include "OSXVideoDecoder.h"
#import "AppStreamWrapper.h"

#import "NALUtils.h"

#import "VDAOGLRenderer.h"

#import "H264ToYuv.h"

#undef LOG_TAG
#define LOG_TAG "OSXVideoDecoder"
#include "log.h"

#define kVDADecodeInfo_Asynchronous     1UL << 0
#define kVDADecodeInfo_FrameDropped     1UL << 1

// Global reference to an xstx module
extern AppStreamWrapper *gAppStreamWrapper;


/**
 * VDADecoderDecode callback
 * Passes the data through to the OSXVideoDecoder instance
 */
void vdaDecoderOutputCallback(void               *decompressionOutputRefCon,
                             CFDictionaryRef    frameInfo,
                             OSStatus           status,
                             uint32_t           infoFlags,
                             CVImageBufferRef   imageBuffer)
{
    OSXVideoDecoder *vidDecoder = (OSXVideoDecoder*)decompressionOutputRefCon;

    vidDecoder->frameDecoded(frameInfo, status, infoFlags, imageBuffer);
}



OSXVideoDecoder::OSXVideoDecoder() :
mSWDelegate(NULL),
mInitFrame(NULL),
mFoundInitFrame(false),
mVDADecoder(NULL),
mUseFFmpeg(false),
mFrameDecodeMutex(PTHREAD_MUTEX_INITIALIZER)
{
}


/**
 * Decode a frame.
 *
 * @param[in] enc Frame to be decoded.
 * @param[out] dec Holder for decoded frame.
 */
XStxResult OSXVideoDecoder::decodeFrame(XStxEncodedVideoFrame *enc, XStxRawVideoFrame *dec)
{
    if (mUseFFmpeg) {
        //If we had to fall back to FFmpeg then pass this to the FFmpeg decoder
        return ffmpegDecoder->decodeFrame(enc, dec);
    }
    if (enc == NULL || dec == NULL)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    OSStatus status;
    OSType inSourceFormat='avc1';
    CFDataRef inAVCCData;

    uint8_t *blockEnd = enc->mData+enc->mDataSize;
    uint8_t *start = enc->mData;

    if (!mFoundInitFrame) {
        //We don't have the initFrame yet

        uint8 *spsStart = NULL;
        uint32_t spsLength = 0;
        BOOL foundSPS = getNALUnit(NAL_TYPE_SPS, start, blockEnd, spsStart, spsLength);

        uint8 *ppsStart = NULL;
        uint32_t ppsLength = 0;
        BOOL foundPPS = getNALUnit(NAL_TYPE_PPS, start, blockEnd, ppsStart, ppsLength);

        if (!foundSPS || !foundPPS) {
            LOGE("Did not find SPS or PPS");
            return XSTX_RESULT_VIDEO_DECODING_ERROR;
        }

        uint8_t extraDataSize = spsLength + ppsLength + 11;//extraData is SPS + PPS + 11 bytes
        uint8_t *extraData = new uint8_t[extraDataSize];
        memset(extraData, 0xFF, extraDataSize); //Make sure extraData is initialized

        //Create the h.264 extradata NAL
        makeExtraData(extraData, spsStart, spsLength, ppsStart, ppsLength);

        //Turn it into a CFData
        inAVCCData = CFDataCreate(kCFAllocatorDefault, extraData, extraDataSize*sizeof(UInt8));

        //Parse the SPS packet
        sps theSPS = parseSPS(spsStart, spsLength);

        //Pull the Width & Height from the SPS
        videoWidth = 0;
        videoHeight = 0;
        getVideoSizeFromSPS(theSPS, videoWidth, videoHeight);


        //Create the VDADecoder
        if( disableHardwareDecode )
        {
            status = 1;
        }
        else
        {
            status = CreateDecoder(videoHeight, videoWidth, inSourceFormat, inAVCCData, &mVDADecoder);
        }

        if (status != kVDADecoderNoErr) {
            //Failed trying to create the decoder so fall back to FFmpeg

            LOGE("VDADecoderCreate failed or disabled. Using FFmpeg");

            //Create and initialize FFmpeg
            ffmpegDecoder = new(std::nothrow) H264ToYuv();
            ffmpegDecoder->init();
            mUseFFmpeg = true;

            //Tell the renderer we are now using FFmpeg
            VDAOGLRenderer *theRenderer = (VDAOGLRenderer *)gAppStreamWrapper->getVideoRenderer();
            theRenderer->_useVDADecoder = false;

            //Now use FFmpeg to decode the frame
            return ffmpegDecoder->decodeFrame(enc, dec);
        }

        //Verify success
        switch (status) {
            case kVDADecoderNoErr:
                //Decoder created we are all good
                LOGE("VDADecoder Created!");
                break;

            case kVDADecoderHardwareNotSupportedErr:
                //The hardware does not support accelerated video services required for hardware decode.
                return XSTX_RESULT_VIDEO_DECODING_ERROR;

                break;
            case kVDADecoderConfigurationError:
                //Invalid or unsupported configuration parameters were specified in VDADecoderCreate
                return XSTX_RESULT_VIDEO_DECODING_ERROR;
                break;
            case kVDADecoderDecoderFailedErr:
                //An error was returned by the decoder layer. This may happen for example because of bitstream/data errors during a decode operation. This error may also be returned from VDADecoderCreate when hardware decoder resources are available on the system but currently in use by another process.
                return XSTX_RESULT_VIDEO_DECODING_ERROR;
                break;
            case -50:
                //Parameter error (bad avcC data?)
                return XSTX_RESULT_VIDEO_DECODING_ERROR;
                break;

            default:
                //Unknown Status
                return XSTX_RESULT_VIDEO_DECODING_ERROR;
                break;
        }

        mFoundInitFrame = true;
    }

    //We have initialized the decoder so try to decode the video frame
    if (mFoundInitFrame)
    {
        CFDataRef frameData = convertFrameToData(enc->mData, enc->mDataSize);

        if (frameData == NULL) {
            //No frame found
            LOGE("No frame found!");
        } else
        {
            // create a CFDictionary to store the frame time stamp
            CFStringRef key = CFSTR("TimeStamp");
            CFNumberRef value = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &enc->mTimestampUs);

            CFDictionaryRef frameDict = CFDictionaryCreate(kCFAllocatorDefault,
                                      (const void **)&key,
                                      (const void **)&value,
                                      1,
                                      &kCFTypeDictionaryKeyCallBacks,
                                      &kCFTypeDictionaryValueCallBacks);


            status = VDADecoderDecode(mVDADecoder, 0, frameData, frameDict);

            switch (status) {
                case kVDADecoderNoErr:
                    //DecoderDecode called successfully
                    break;

                case kVDADecoderDecoderFailedErr:
                    //An error was returned by the decoder layer. This may happen for example because of bitstream/data errors during a decode operation. This error may also be returned from VDADecoderCreate when hardware decoder resources are available on the system but currently in use by another process.
                    //This is probably fine unless it is happening every frame
                    LOGE("VDADecoder Failed Error");
                    break;


                case kVDADecoderHardwareNotSupportedErr:
                    //The hardware does not support accelerated video services required for hardware decode.
                    LOGE("VDADecoderHardware not supported");
                    break;

                case kVDADecoderConfigurationError:
                    //Invalid or unsupported configuration parameters were specified in VDADecoderCreate
                    LOGE("VDADecoder Configuration Error");
                    break;

                case -50:
                    //Parameter error (bad avcC data?)
                    LOGE("VDADecoder Param Error");
                    break;

                default:
                    //Unknown Status
                    LOGE("VDADecoder Unknown Error");
                    break;
            }
        }


        if (frameData != NULL)
        {
            CFRelease(frameData);
        }
    }

    //Return a dummy decoded buffer with the right timestamp
    static uint8_t dummy=0;
    dec->mWidth = videoWidth;
    dec->mHeight = videoHeight;
    dec->mTimestampUs = enc->mTimestampUs;
    dec->mPlanes[0]=&dummy;
    dec->mPlanes[1]=&dummy;
    dec->mPlanes[2]=&dummy;
    dec->mStrides[0]=dec->mWidth;
    dec->mStrides[1]=dec->mWidth;
    dec->mStrides[2]=dec->mWidth;
    dec->mBufferSizes[0]=1;
    dec->mBufferSizes[1]=1;
    dec->mBufferSizes[2]=1;

    return XSTX_RESULT_OK;
}

/**
 * Initialize the decoder.
 *
 * @return XSTX_RESULT_OK on success.
 */
XStxResult OSXVideoDecoder::init()
{
    //Nothing to do on init all our work happens when the stream actually starts
    return XSTX_RESULT_OK;
}

/**
 * Creates the VDADecoder
 *
 * @return OSStatus result from calling VDADecoderCreate
 */
OSStatus OSXVideoDecoder::CreateDecoder(SInt32 inHeight, SInt32 inWidth,
                       OSType inSourceFormat, CFDataRef inAVCCData,
                       VDADecoder *decoderOut)
{
    OSStatus status;

    CFMutableDictionaryRef decoderConfiguration = NULL;
    CFMutableDictionaryRef destinationImageBufferAttributes = NULL;
    CFDictionaryRef ioSurfacePropDictionary;

    CFNumberRef height = NULL;
    CFNumberRef width= NULL;
    CFNumberRef sourceFormat = NULL;
    CFNumberRef pixelFormat = NULL;

    // source must be H.264
    if (inSourceFormat != 'avc1') {
        LOGE("Source format is not H.264!");
        return paramErr;
    }

    // the avcC data chunk from the bitstream must be present
    if (inAVCCData == NULL) {
        LOGE("avc1 decoder configuration data cannot be NULL!");
        return paramErr;
    }

    // create a CFDictionary describing the source material for decoder configuration
    decoderConfiguration = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                     4,
                                                     &kCFTypeDictionaryKeyCallBacks,
                                                     &kCFTypeDictionaryValueCallBacks);

    height = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inHeight);
    width = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inWidth);
    sourceFormat = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &inSourceFormat);

    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_Height, height);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_Width, width);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_SourceFormat, sourceFormat);
    CFDictionarySetValue(decoderConfiguration, kVDADecoderConfiguration_avcCData, inAVCCData);


    // create a CFDictionary describing the wanted destination image buffer
    destinationImageBufferAttributes = CFDictionaryCreateMutable(kCFAllocatorDefault,
                                                                 2,
                                                                 &kCFTypeDictionaryKeyCallBacks,
                                                                 &kCFTypeDictionaryValueCallBacks);

    OSType cvPixelFormatType = kCVPixelFormatType_422YpCbCr8;
    pixelFormat = CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &cvPixelFormatType);
    ioSurfacePropDictionary = CFDictionaryCreate(kCFAllocatorDefault,
                                         NULL,
                                         NULL,
                                         0,
                                         &kCFTypeDictionaryKeyCallBacks,
                                         &kCFTypeDictionaryValueCallBacks);

    CFDictionarySetValue(destinationImageBufferAttributes, kCVPixelBufferPixelFormatTypeKey, pixelFormat);
    CFDictionarySetValue(destinationImageBufferAttributes,
                         kCVPixelBufferIOSurfacePropertiesKey,
                         ioSurfacePropDictionary);

    // create the hardware decoder object
    status = VDADecoderCreate(decoderConfiguration,
                              destinationImageBufferAttributes,
                              (VDADecoderOutputCallback*)vdaDecoderOutputCallback,
                              (void *) this,
                              decoderOut);

    if (kVDADecoderNoErr != status) {
        LOGE("VDADecoderCreate failed. err: %d", status);
    }


    //Cleanup
    if (decoderConfiguration)
    {
        CFRelease(decoderConfiguration);
    }
    if (destinationImageBufferAttributes)
    {
        CFRelease(destinationImageBufferAttributes);
    }
    if (ioSurfacePropDictionary)
    {
        CFRelease(ioSurfacePropDictionary);
    }

    return status;
}

/**
 * Converts the frame to the format needed for VDADecoder (4-byte size values
 * instead of 00 00 01 NAL headers) and returns it as a CFData
 *
 * @param[in] pointer to the beginning of the frame data
 * @param[in] size of the frame data passed in
 */
CFDataRef OSXVideoDecoder::convertFrameToData ( uint8_t *dataStart, uint32_t dataSize)
{
    uint8_t *newFrameData = NULL;
    uint32_t newFrameLength = 0;

    pthread_mutex_lock(&mFrameDecodeMutex);
    bool convertedFrame = convertToSizeEncodedVideoFrame(dataStart, dataSize, newFrameData, newFrameLength);

    if (!convertedFrame || newFrameLength <= 0) {
        LOGE("Could not convert frame");
        pthread_mutex_unlock(&mFrameDecodeMutex);
        return NULL;
    }

    CFDataRef frameData = NULL;

    frameData = CFDataCreate(kCFAllocatorDefault, newFrameData, newFrameLength);
    pthread_mutex_unlock(&mFrameDecodeMutex);

    return frameData;
}


/**
 * Callback for getting the decoded frame from the VDADecoder
 *
 * @param[in] frameInfo dictionary created when frame was passed to VDADecode
 * @param[in] status set by VDADecode
 * @param[in] infoFlags set by VDADecode
 * @param[in] imageBuffer CVImage buffer holding the decoded frame
 */
void OSXVideoDecoder::frameDecoded(CFDictionaryRef    frameInfo,
                                   OSStatus           status,
                                   uint32_t           infoFlags,
                                   CVImageBufferRef   imageBuffer)
{
    if (imageBuffer == NULL) {
        LOGE("Received a NULL imageBuffer");
        if (kVDADecodeInfo_FrameDropped & infoFlags) {
            LOGE("Frame was dropped by decoder");
        }
        return;
    }

    if (CVPixelBufferGetPixelFormatType(imageBuffer) != '2vuy') {
        LOGE("Pixel buffer is in wrong format");
        return;
    }

    //Give the decoded frame to the renderer (if it exists)
    VDAOGLRenderer *theRenderer = (VDAOGLRenderer *)gAppStreamWrapper->getVideoRenderer();
    if (theRenderer)
    {
        theRenderer->setCVPixelBuffer(imageBuffer);
    }
}


bool OSXVideoDecoder::receivedClientConfiguration(const XStxClientConfiguration* config) {
    
    // YUV420 or YUV444
    mChromaSampling = config->mChromaSampling;
    LOGV("Received chroma sampling option: %d", static_cast<int>(mChromaSampling));
    if (mChromaSampling == XSTX_CHROMA_SAMPLING_YUV420)
    {
        LOGV("Chroma Sampling: YUV420");
    } else if (mChromaSampling == XSTX_CHROMA_SAMPLING_YUV444)
    {
        LOGV("Chroma Sampling: YUV444");
    } else
    {
        LOGV("Chroma Sampling Unknown");
    }
    // no log ?? LOG("Received chroma sampling option: %d", static_cast<int>(mChromaSampling));
    return isChromaSamplingSupported(mChromaSampling);
}

bool OSXVideoDecoder::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420
    || chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
}
