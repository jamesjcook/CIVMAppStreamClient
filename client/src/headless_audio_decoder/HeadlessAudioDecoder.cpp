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


#include <assert.h>
#include <new>
#include <stdio.h>

#include "HeadlessAudioDecoder.h"
#include "log.h"
/** Constructor */
HeadlessAudioDecoder::HeadlessAudioDecoder()
{
}

/** Destructor */
HeadlessAudioDecoder::~HeadlessAudioDecoder()
{
}

/** Initialize Opus decoder */
XStxResult HeadlessAudioDecoder::start()
{
    return XSTX_RESULT_OK;
}

/**
 * Decode Opus frame into Pcm frame
 * @param[in] in Opus encoded audio frame
 * @param[out] out holder for decoded audio frame
 */
XStxResult HeadlessAudioDecoder::decodeFrame(
    XStxEncodedAudioFrame *in, XStxRawAudioFrame *out)
{
    // check parameters
    if (in == NULL || out == NULL)
    {
        LOGE("HeadlessAudioDecoder: invalid arguments");
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    // If the data size is 0 and mData is NULL, the
    // opus decoder produces a packet loss concealment (PLC) frame.
    if (in->mDataSize == 0 && NULL != in->mData )
    {
        LOGE("HeadlessAudioDecoder: invalid arguments");
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    
    if (out->mBufferSize < 480 * 2)
    {
        // invalid buffer
        LOGE("HeadlessAudioDecoder: decoding errors");
        return XSTX_RESULT_AUDIO_DECODING_ERROR;
    }
    // successfully decoded
    out->mTimestampUs = in->mTimestampUs;
    static uint64_t count = 0;
    if(count % 400 == 0)
    {
            LOGV("Received Audio Frame size:%u timeStamp:%llu", in->mDataSize, in->mTimestampUs);
    }
    count++;
    return XSTX_RESULT_OK;
}


