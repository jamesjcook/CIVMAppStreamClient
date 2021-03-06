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


#include "VideoPipeline.h"
#include "OSXVideoDecoder.h"
#include "VDAOGLRenderer.h"

#undef LOG_TAG
#define LOG_TAG "VideoPipeline"
#include "log.h"

VideoDecoder * newVideoDecoder()
{
    bool shouldDisableVDA = [[NSUserDefaults standardUserDefaults] boolForKey:@"disableVDADecode"];
    
    OSXVideoDecoder* decoder = new(std::nothrow) OSXVideoDecoder();
    
    if(shouldDisableVDA)
    {
        decoder->disableHardwareDecode = true;
    }
    if (!decoder)
    {
        LOGE("Failed to create Decoder\n");
        return NULL;
    }
    return decoder ;
}

VideoRenderer *newVideoRenderer()
{
    VideoRenderer *renderer = new(std::nothrow) VDAOGLRenderer();
    
    if (!renderer)
    {
        return NULL;
    }
    
    return renderer;
}

extern "C"
XStxResult pipelineVideoDecoderGetCapabilities(void *context,
                                               XStxVideoDecoderCapabilities *decoderCapabilities)
{
    if (NULL == context || NULL == decoderCapabilities)
    {
        assert(context != NULL);
        assert(decoderCapabilities != NULL);
        
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }
    decoderCapabilities->mProfile = XSTX_H264_PROFILE_HIGH444;
    
    /* Level 5.0 means the decoder can decode 720p60 with a dpb
     * size of 20 frames enabling use of long term reference frames
     * to recover from lost packets with RTT up to 280 ms.
     *
     * It should also be capable of 1080p60 with dpb 13
     */
    decoderCapabilities->mLevel = XSTX_H264_LEVEL_5_0;
    
    decoderCapabilities->mSupportsLtr = true;
    decoderCapabilities->mSupportsLtrWithPartialFrames = true;
    decoderCapabilities->mSupportsPartialFrameIfNoLtr = true;
    
    return XSTX_RESULT_OK;
}