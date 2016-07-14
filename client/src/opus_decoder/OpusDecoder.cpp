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

#include "OpusDecoder.h"

/** Constructor */
OpusDecoder::OpusDecoder()
    :
      mOpusDecoderContext(NULL)
{
}

/** Destructor */
OpusDecoder::~OpusDecoder()
{

    if (NULL != mOpusDecoderContext)
    {
        opus_decoder_destroy(mOpusDecoderContext);
        mOpusDecoderContext = NULL;
    }
}

/** Initialize Opus decoder */
XStxResult OpusDecoder::start()
{
    // create decoder
    int opusInitError = 0;
    mOpusDecoderContext = opus_decoder_create(SAMPLING_RATE, NUM_CHANNELS,
                                           &opusInitError);
    if (mOpusDecoderContext == NULL)
    {
        // failed to instantiate decoder
        return XSTX_RESULT_OUT_OF_MEMORY;
    }
    return XSTX_RESULT_OK;
}

/**
 * Decode Opus frame into Pcm frame
 * @param[in] in Opus encoded audio frame
 * @param[out] out holder for decoded audio frame
 */
XStxResult OpusDecoder::decodeFrame(
    XStxEncodedAudioFrame *in, XStxRawAudioFrame *out)
{
    // check parameters
    if (in == NULL || out == NULL)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    // If the data size is 0 and mData is NULL, the
    // opus decoder produces a packet loss concealment (PLC) frame.
    if (in->mDataSize == 0 && NULL != in->mData )
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    // check if decoder is ready
    if (mOpusDecoderContext == NULL)
    {
        return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
    }

    // decode
    int numSamples = opus_decode(
        mOpusDecoderContext,
        in->mData,
        in->mDataSize,
        (opus_int16 *)out->mData,
        SAMPLES_PER_FRAME,
        0);

    if (numSamples <= 0)
    {
        // decoding failed
        return XSTX_RESULT_AUDIO_DECODING_ERROR;
    }

    // successfully decoded
    out->mTimestampUs = in->mTimestampUs;
    out->mDataSize = numSamples * NUM_CHANNELS * (16 / 8);

    return XSTX_RESULT_OK;
}


