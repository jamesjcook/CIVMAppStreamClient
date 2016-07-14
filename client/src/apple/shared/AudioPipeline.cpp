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

#include "AudioPipeline.h"
#include "OpusDecoder.h"
#include "AudioRenderer.h"
#include "CoreAudioRenderer.h"

#undef LOG_TAG
#define LOG_TAG "AudioPipeline"
#include "log.h"


AudioDecoder * newAudioDecoder()
{
    AudioDecoder* decoder = new(std::nothrow) OpusDecoder();
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
    // note: this variable declared static here as CoreAudio initialization
    // is preferably done on an application session level,
    // not per each streaming session connect
    
    static std::shared_ptr<CoreAudioRenderer> renderer =
        std::shared_ptr<CoreAudioRenderer>(new(std::nothrow)
                                           CoreAudioRenderer(framePool,clientHandle));

    //Make sure the mClientHandle is properly set
    renderer->mClientHandle = clientHandle;
    
    return (AudioRenderer*) renderer.get();
}

