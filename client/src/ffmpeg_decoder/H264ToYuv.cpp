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


#include "AvHelper.h"
#include "H264ToYuv.h"

#undef LOG_TAG
#define LOG_TAG "H264ToYuv"

#include "log.h"

/** Constructor */
H264ToYuv::H264ToYuv()
    : mCodecContext(NULL)
    , mChromaSampling(XSTX_CHROMA_SAMPLING_YUV420)
{
}

/** Destructor */
H264ToYuv::~H264ToYuv()
{
    if (mCodecContext != NULL)
    {
        // free up FFmpeg codec context
        avcodec_close(mCodecContext);
        av_freep(&mCodecContext);
        av_free_packet(&mAvPacket);
    }

    // terminate logger
    AvHelper::terminate();
}

/** initialize */
XStxResult H264ToYuv::init()
{

    // start logger
    AvHelper::initialize();

    // get context
    mCodecContext = avcodec_alloc_context3(NULL);

    if (NULL == mCodecContext)
    {
        // failed to initialize context
        return XSTX_RESULT_VIDEO_DECODING_ERROR;
    }

    // Let's work around these bugs.
    mCodecContext->workaround_bugs = 1;

    // using video
    mCodecContext->codec_type           = AVMEDIA_TYPE_VIDEO;
    // H264 encoding
    mCodecContext->codec_id             = CODEC_ID_H264;
    // YUV420P format
    mCodecContext->pix_fmt              = mChromaSampling == XSTX_CHROMA_SAMPLING_YUV420
        ? PIX_FMT_YUV420P : PIX_FMT_YUV444P;

    AVCodec *codec = avcodec_find_decoder(CODEC_ID_H264);

    if (NULL == codec)
    {
        // failed to find decoder
        return XSTX_RESULT_VIDEO_DECODING_ERROR;
    }

    if (avcodec_open2(mCodecContext, codec, NULL) < 0)
    {
        return XSTX_RESULT_VIDEO_DECODING_ERROR;
    }

    av_init_packet(&mAvPacket);

    // successfully initialized
    return XSTX_RESULT_OK;
}

/**
 * Fetch A/V frame associated with given encoded video frame
 * @param[in] dec encoded video frame
 * @param[out] avFrame associated AV frame
 */
XStxResult H264ToYuv::getAvFrame(XStxRawVideoFrame *dec, AVFrame *&avFrame)
{
    // check our A/V frame pool first
    std::map<uint8_t *, AVFrame *>::iterator it =
        planeToAvFrame.find(dec->mPlanes[0]);
    if (it != planeToAvFrame.end())
    {
        // found it
        avFrame = it->second;
        return XSTX_RESULT_OK;
    }

    // otherwise, allocate new frame
    avFrame = avcodec_alloc_frame();
    if (avFrame == NULL)
    {
        // failed to allocate new frame
        LOGE("Failed to allocate new frame: %s",
             XStxResultGetDescription(XSTX_RESULT_OUT_OF_MEMORY));
        return XSTX_RESULT_OUT_OF_MEMORY;
    }
    avcodec_get_frame_defaults(avFrame);
    return XSTX_RESULT_OK;
}

/**
 * Decode frame
 * @param[in] enc frame to be decoded
 * @param[out] dec holder for decoded frame
 */
XStxResult H264ToYuv::decodeFrame(
    XStxEncodedVideoFrame *enc,
    XStxRawVideoFrame *dec)
{
    if (enc == NULL || dec == NULL)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    // fetch A/V frame from the pool
    AVFrame *avFrame = NULL;
    if (getAvFrame(dec, avFrame) != XSTX_RESULT_OK)
    {
        return XSTX_RESULT_OUT_OF_MEMORY;
    }

    // decode frame
    uint8_t *planeIndex = dec->mPlanes[0];

    mAvPacket.data = enc->mData;
    mAvPacket.size = enc->mDataSize;

    int gotPicture = 0;
    int avResult = avcodec_decode_video2(
        mCodecContext,
        avFrame,
        &gotPicture,
        &mAvPacket);

    if (avResult >= 0 && gotPicture != 0)
    {
        // decoding succeeded
        if (planeIndex != avFrame->data[0])
        {
            // our indexing changed, update our map
            if (planeIndex != 0)
            {
                planeToAvFrame.erase(planeIndex);
            }
            planeToAvFrame[avFrame->data[0]] = avFrame;
        }

        // populate decoded frame
        for (int i = 0; i < 3; i++)
        {
            dec->mPlanes[i] = avFrame->data[i];
            dec->mStrides[i] = avFrame->linesize[i];
        }
        dec->mWidth = mCodecContext->width;
        dec->mHeight = mCodecContext->height;
        dec->mTimestampUs = enc->mTimestampUs;

        // successfully decoded frame
        return XSTX_RESULT_OK;
    }

    return XSTX_RESULT_VIDEO_DECODING_ERROR;
}

bool H264ToYuv::receivedClientConfiguration(const XStxClientConfiguration* config) {
    
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

bool H264ToYuv::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420
        || chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
}
