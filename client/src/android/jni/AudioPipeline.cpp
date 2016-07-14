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

#include "AudioPipeline.h"
#include "AndroidOpusDecoder.h"
#include "AndroidAudioRenderer.h"

#undef LOG_TAG
#define LOG_TAG "AudioPipeline"
#include "log.h"

AudioDecoder * newAudioDecoder()
{
    AudioDecoder* decoder = new(std::nothrow) AndroidOpusDecoder();
    if (!decoder)
    {
        LOGE("Failed to create Decoder\n");
        return NULL;
    }
    return decoder ;
}

AudioRenderer *newAudioRenderer(mud::FixedSizePool<XStxRawAudioFrame> &framePool,
        XStxClientHandle clientHandle)
{
    AudioRenderer * renderer =
        new(std::nothrow) AndroidAudioRenderer(framePool,clientHandle);
    if (!renderer)
    {
        return NULL;
    }

    return renderer;
}

