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


#ifndef _included_OpusToPcm_h
#define _included_OpusToPcm_h


#include "AudioDecoder.h"

extern "C"
{
#include <opus/opus.h>
};

#include "XStx/common/XStxAPI.h"

/**
 * The Audio Decoder implementation using Opus; decodes Opus samples into
 * PCM samples.
 */
class OpusDecoder : public AudioDecoder
{

public:

    /** Constructor */
    OpusDecoder();

    /** Destructor */
    virtual ~OpusDecoder();

    /** Initialize Opus decoder */
    virtual XStxResult start();

    /**
     * Decode Opus frame into Pcm frame
     * @param[in] in Opus encoded audio frame
     * @param[out] out holder for decoded audio frame
     */
    virtual XStxResult decodeFrame(XStxEncodedAudioFrame *in, XStxRawAudioFrame *out);

private:
    /** opus decoder context for decoding audio */
    OpusDecoder *mOpusDecoderContext;

    // constants as defined by headers in XStxClientAPI.h:DecodeAudioFrame
    static const uint32_t NUM_CHANNELS = 2;
    static const uint32_t SAMPLING_RATE = 48000;
    static const uint32_t SAMPLES_PER_FRAME = 480;
};


#endif
