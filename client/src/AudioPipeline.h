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

#ifndef _included_AudioPipeline_h
#define _included_AudioPipeline_h

/**
 * @file AudioPipeline.h
 *
 * A cross-platform header that defines functions that are implemented on
 * each platformt o create an AudioDecoder and an AudioRenderer.
 */

#include <MUD/memory/FixedSizePool.h>

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>

class AudioDecoder;
class AudioRenderer;

/**
 * Create a platform- and application-specific AudioDecoder.
 *
 * @return A new AudioDecoder. Should be deleted on cleanup.
 */
AudioDecoder *newAudioDecoder();

/**
 * Create a platform- and application-specific AudioRenderer.
 *
 * @param framePool The audio frame pool
 * @param clientHandle
 *                  The active XStxClientHandle.
 *
 * @return A new AudioRenderer. Should be deleted on cleanup.
 */
AudioRenderer *newAudioRenderer(
                  mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                  XStxClientHandle clientHandle);

#endif // _included_AudioPipeline_h
