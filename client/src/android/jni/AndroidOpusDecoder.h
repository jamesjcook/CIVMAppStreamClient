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

#ifndef _included_AndroidOpusDecoder_h
#define _included_AndroidOpusDecoder_h

#include "opus_decoder/OpusDecoder.h"

#include "XStx/common/XStxAPI.h"

/**
 * The Audio Decoder implementation using Opus; decodes Opus samples into
 * PCM samples. This Android implementation also resamples the audio.
 */
class AndroidOpusDecoder : public OpusDecoder
{

public:
	virtual ~AndroidOpusDecoder() {}

    /**
     * Decode Audio frame into Pcm frame
     * @param[in] in encoded audio frame
     * @param[out] out holder for decoded audio frame
     */
    virtual XStxResult decodeFrame(XStxEncodedAudioFrame *in, XStxRawAudioFrame *out);
};


#endif
