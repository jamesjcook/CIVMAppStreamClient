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


#ifndef _included_VideoPipeline_h
#define _included_VideoPipeline_h

#include <assert.h>
#include "XStx/common/XStxResultAPI.h"

/**
 * @file VideoPipeline.h
 *
 * A cross-platform header that defines functions that are implemented on
 * each platformt o create an VideoDecoder and an VideoRenderer.
 */

class VideoDecoder;
class VideoRenderer;

/**
 * Create a platform- and application-specific VideoDecoder.
 *
 * @return A new VideoDecoder. Should be deleted on cleanup.
 */
VideoDecoder *newVideoDecoder();

/**
 * Create a platform- and application-specific VideoRenderer.
 *
 * @return A new VideoRenderer. Should be deleted on cleanup.
 */
VideoRenderer *newVideoRenderer();

/**
 * Set the video decoder capabilities based on the current video pipeline.
 *
 * @param context             Always null
 * @param decoderCapabilities An object to fill with the appropriate caps.
 *
 * @return XStxResult         Should return XSTX_RESULT_OK unless there's
 *                            a problem.
 */
#ifdef __cplusplus
extern "C"
#endif
XStxResult pipelineVideoDecoderGetCapabilities(
    void *context,
    XStxVideoDecoderCapabilities *decoderCapabilities);

#endif // _included_VideoPipeline_h
