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

#include "DummyAudioRenderer.h"

#undef LOG_TAG
#define LOG_TAG "AudioRenderer"

#include "log.h"

/**
 * The DummyAudio based audio renderer.
 */
/** Constructor */
DummyAudioRenderer::DummyAudioRenderer(
                        mud::FixedSizePool<XStxRawAudioFrame> &framePool,
                        XStxClientHandle clientHandle) 
                            : AudioRenderer(framePool, clientHandle),
                              mDummyAudioStream(NULL),
                              mTimeoutMarginInMs(INITIAL_TIMEOUT_MARGIN_MS),
                              mAudioIsPlaying(false),
                              mFrame(NULL),
                              mPlaybackPosInFrame(0),
                              mDidInit(false),
                              mShouldStop(false)
{
}


/** Destructor */
DummyAudioRenderer::~DummyAudioRenderer()
{
    // If we've got a DummyAudioStream, Pa_Initialize has succeeded,
    // and Pa_OpenStream has succeeded. We need to close the stream,
    // and call Pa_Terminate to clean up. If DummyAudioStream is NULL,
    // we don't need to call Pa_Terminate. It's either already been
    // called or Pa_Initialize hasn't been called.
	/*
    if (NULL != mDummyAudioStream) 
    {
        PaError err = Pa_CloseStream(mDummyAudioStream);

        if(paNoError != err) 
        {
            LOGW("Failed to close port audio stream %d", err);
        }

        err = Pa_Terminate();

        if(paNoError != err)
        {
            LOGW("Failed to terminate port audio %d", err);
        }
    }
	*/
}

/**
 * Acquire an audio frame to render.
 */
XStxRawAudioFrame* DummyAudioRenderer::popFrame(int delay, int msBuffer)
{
    XStxRawAudioFrame * frame = NULL;

    XStxResult result = XStxGetNextAudioFrame(
                                    mClientHandle, 
                                    &frame, 
                                    delay, 
                                    msBuffer);

    if (result != XSTX_RESULT_OK)
    {
        LOGV("Failed to get audio frame! %d",result);

        return NULL;
    }

    return frame;
}

/**
 * Start an audio stream. After this, audio stream callbacks will
 * start to request audio to play back.
 */
XStxResult DummyAudioRenderer::start()
{
    if (!mDidInit)
    {
        if (!initializeDummyAudio())
        {
            return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
        }
        mDidInit = true;
    }

    if (!mAudioIsPlaying)  
    {
		/*
        PaError err = Pa_StartStream(mDummyAudioStream);
        if(paNoError != err) 
        {
            LOGW("Failed to start port audio stream");
            return XSTX_RESULT_AUDIO_RENDERING_ERROR;
        }
		*/
        mAudioIsPlaying = true;
    }

    return XSTX_RESULT_OK;
}

/**
 * Pause the audio renderer.
 */
void DummyAudioRenderer::pause(bool pause)
{
	LOGV("AndroidAudioRenderer::pause");
    mPaused = pause;
}

/**
 * Stop the audio renderer.
 */
void DummyAudioRenderer::stop()
{
    mud::ScopeLock sl(mStopLock);
    mShouldStop = true;
	/*
    if (mDummyAudioStream != NULL) 
    {
        Pa_StopStream(mDummyAudioStream);
    }
	*/
}

/**
 * Initialize DummyAudio stream
 */
bool DummyAudioRenderer::initializeDummyAudio()
{
	/*
    PaError err = -1;
    PaStreamParameters paStreamParams;

    //err = Pa_Initialize();
    if(paNoError != err) 
    {
        LOGW("Failed to initialzie port audio %d ", err);
        // failed to initialize
        return false;
    }

    paStreamParams.device = Pa_GetDefaultOutputDevice();
    paStreamParams.channelCount = NUM_CHANNELS;
    // assuming BitsPerSample is 16
    paStreamParams.sampleFormat = paInt16;
    paStreamParams.suggestedLatency = (double) SUGGESTED_PA_LATENCY_MS 
                                                / 1000; // in seconds.
    paStreamParams.hostApiSpecificStreamInfo = NULL;

    // open DummyAudio stream with configured parameters
    err = Pa_OpenStream(
                &mDummyAudioStream,
                NULL,
                &paStreamParams,
                SAMPLING_RATE,
                    // we want DummyAudio to give us Buffers of the right size, 
                    // otherwise we will be more likely to hear glitches.
                (SAMPLING_RATE * NUM_MS_PER_FRAME) / 1000,
                paClipOff,
                paStreamCallback,
                this);

    if(paNoError != err) 
    {
        LOGW("Failed to open port audio stream");
        // failed to open DummyAudio stream
        Pa_Terminate();
        return false;
    }
	*/
    // successfully initialized DummyAudio stream
    return true;

}

/**
 * The fillPABuffer method
 */
bool DummyAudioRenderer::fillPABuffer(
                            int16_t* buffer, 
                            int numSamplesPerChannel,
                            const PaStreamCallbackTimeInfo* timeInfo)
{

	/*
    mud::ScopeLock sl(mStopLock);
    if (mShouldStop) {
        return false;
    }

    uint32_t  initialTimeUntilDeadlineInMs = static_cast<uint32_t>((timeInfo->outputBufferDacTime 
                                                                        - timeInfo->currentTime) * 1000.0);

    mud::TimeVal timeWaited = mud::TimeVal::mono();
    
    // Loop until we've written out all the samples that are requested, or
    // until we've run out of sample received from the decoder.

    uint32_t numSamplesRequested = numSamplesPerChannel * NUM_CHANNELS;
    uint32_t numSamplesWritten = 0;

    while((numSamplesWritten < numSamplesRequested))
    {
        mud::TimeVal timeRemaining =  
            mud::TimeVal::fromMilliSeconds(initialTimeUntilDeadlineInMs) - 
            timeWaited.elapsedMono();
        uint32_t timeout = 0;
        if(timeRemaining.toMilliSeconds() > mTimeoutMarginInMs)
        {
            timeout = (uint32_t)(timeRemaining.toMilliSeconds()) - mTimeoutMarginInMs;
        }

        if(mFrame == NULL) 
        {
            mFrame = popFrame(
                static_cast<uint32_t>(timeRemaining.toMilliSeconds()), 
                timeout);

            if (mFrame == NULL)
            {
                return false;
            }

            mPlaybackPosInFrame = 0;
        }

        if(mPlaybackPosInFrame >= mFrame->mDataSize)
        {
            // nothing left in this frame that we can read from
            mPlaybackPosInFrame = 0;
            mFramePool.recycleElement(mFrame);
            mFrame = NULL;
            continue;
        }
        
        uint32_t maxSamplesInFrame = (mFrame->mDataSize - mPlaybackPosInFrame) /
            BYTES_PER_SAMPLE;
        uint32_t maxSamplesCanWrite = numSamplesRequested - numSamplesWritten;
        uint32_t numSamplesToTake = maxSamplesInFrame > maxSamplesCanWrite ?
            maxSamplesCanWrite : maxSamplesInFrame;

        memcpy(
            (void*)buffer,
            (void*)((mFrame)->mData +
                mPlaybackPosInFrame),
            numSamplesToTake * BYTES_PER_SAMPLE);
       
        buffer += numSamplesToTake;
        mPlaybackPosInFrame += numSamplesToTake * BYTES_PER_SAMPLE;
        numSamplesWritten += numSamplesToTake;
        if(mPlaybackPosInFrame >= mFrame->mDataSize) 
        {
            mFramePool.recycleElement(mFrame);
            mFrame=NULL;
            mPlaybackPosInFrame = 0;
        }
    }
	*/
    return true;
}

/** DummyAudio callback */
int DummyAudioRenderer::paStreamCallback(
                            const void *inputBuffer, 
                            void *outputBuffer,
                            unsigned long framesPerBuffer,
                            const PaStreamCallbackTimeInfo* timeInfo,
                            PaStreamCallbackFlags statusFlags,
                            void *userData )
{
	/*
    DummyAudioRenderer* renderer = (DummyAudioRenderer*) userData;
	
    int16_t* out = (int16_t*) outputBuffer;

    if (renderer->fillPABuffer(out, framesPerBuffer, timeInfo))
    {
        return paContinue;
    }
	*/
    return paAbort;
}
