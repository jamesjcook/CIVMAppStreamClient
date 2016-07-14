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



#ifndef _included_AudioRenderer_h
#define _included_AudioRenderer_h

#include <XStx/common/XStxAPI.h>
#include <XStx/client/XStxClientAPI.h>

#include <MUD/memory/FixedSizePool.h>

/**
 * The abstract base class of an audio renderer.
 */
class AudioRenderer
{

public:

    /** Constructor */
    AudioRenderer(mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                  XStxClientHandle clientHandle);

    /** Destructor */
    virtual ~AudioRenderer();

    /**
     * Recycle an audio frame.
     *
     * @param[in] frame  Frame to recycle.
     */
    virtual void recycleFrame(XStxRawAudioFrame *frame);

    /**
     * Acquire an audio frame to render.
     *
     * @param[in] msBuffer How much time we can wait for the next frame of
     *                 audio to arrive.
     *
     * @return The frame to render, or NULL if none is available.
     */
    virtual XStxRawAudioFrame* popFrame(int delay, int msBuffer)=0;

    /**
     * Start an audio stream. After this, audio stream callbacks will
     * start to request audio to play back.
     */
    virtual XStxResult start()=0;

    /**
     * Pause the audio renderer.
     *
     * @param pause  True to pause; false to resume.
     */
    virtual void pause( bool pause )=0;

    /**
     * Stop the audio renderer.
     */
    virtual void stop()=0;

protected:
    // Pool for audio frames
    mud::FixedSizePool<XStxRawAudioFrame> &mFramePool;

    // The client handle
    XStxClientHandle mClientHandle;
};

#endif //_included_AudioRenderer_h

