/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
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

#include "AndroidOpusDecoder.h"
#include "OpenSLAudio.h"

#include "AudioModule.h"

/**
 * Decode Opus frame into Pcm frame
 * @param[in] in Opus encoded audio frame
 * @param[out] out holder for decoded audio frame
 */
XStxResult AndroidOpusDecoder::decodeFrame(
    XStxEncodedAudioFrame *in, XStxRawAudioFrame *out)
{
    // check parameters
    if (in == NULL || out == NULL)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    static shared_ptr<uint8_t> buffer(new uint8_t[AudioModule::getMaxSize()]);
    static XStxRawAudioFrame tempFrame =
    {
        sizeof(XStxRawAudioFrame),
        AudioModule::getMaxSize(),
        0,
        buffer.get(),
        0
    };

    XStxResult result = OpusDecoder::decodeFrame(in,&tempFrame);

    if (result!=XSTX_RESULT_OK)
    {
        return result;
    }

    out->mDataSize = AudioDownsample(&tempFrame,out->mData);
    out->mTimestampUs = tempFrame.mTimestampUs ;

    return XSTX_RESULT_OK;
}


