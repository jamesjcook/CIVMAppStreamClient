/*
 * NOTICE: Amazon has modified this file from the original.
 * Modifications Copyright (C) 2013-2014 Amazon.com, Inc. or its
 * affiliates. All Rights Reserved.
 *
 * Modifications are licensed under the Amazon Software License
 * (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *  OR CONDITIONS OF ANY KIND, express or implied. See the License for
 *  the specific language governing permissions and limitations under the License.
 *
 * Original source:
 *   https://developer.android.com/tools/sdk/ndk/index.html
 */

/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "OpenSLAudio.h"

#include <assert.h>
#include <queue>
#include <string.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include "AndroidAudioRenderer.h"

#undef LOG_TAG
#define LOG_TAG "OpenSLAudio"
#include "log.h"


mud::SimpleLock gCallbackLock;

// engine interfaces
static SLObjectItf gEngineObject = NULL;
static SLEngineItf gEngineEngine = NULL;

// output mix interfaces
static SLObjectItf outputMixObject = NULL;

// buffer queue player interfaces
static SLObjectItf bqPlayerObject = NULL;
static SLPlayItf bqPlayerPlay;
static SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;
static SLMuteSoloItf bqPlayerMuteSolo;
static SLVolumeItf bqPlayerVolume;

static AudioRenderer *gAudioRenderer = NULL;
static bool gQueuedAudio = false;

static std::queue<XStxRawAudioFrame *> gFrameFreeQueue;

static int gSkip = 0;

// Size of the "silence" packet. Any time a real
// audio frame isn't available, will play this many
// bytes of silence.
#define SILENCE_PACKET_SIZE 1920

uint64_t lastCall = 0;

extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include <libswresample/swresample.h>
}


struct SwrContext *gSwrContext = NULL;

#define FRAME_COUNT 6


// this callback handler is called every time a buffer finishes playing
void bufferQueuePlayerCallback(SLAndroidSimpleBufferQueueItf bq, void *context)
{
    // Only let one caller in at a time.
    mud::ScopeLock scope(gCallbackLock);

    if (!gAudioRenderer)
    {
        // we've been stopped. Quit!
        return;
    }

    // Only start recycling when the queue is full.
    if (gFrameFreeQueue.size() >= FRAME_COUNT)
    {
        XStxRawAudioFrame *toRecycle = gFrameFreeQueue.front();
        gFrameFreeQueue.pop();
        gAudioRenderer->recycleFrame(toRecycle);
    }

    assert(bq == bqPlayerBufferQueue);
    assert(NULL == context);

    static uint8_t nullBuf[SILENCE_PACKET_SIZE] = { 0 };
    static XStxRawAudioFrame nullFrame =
    {
        sizeof(XStxRawAudioFrame), SILENCE_PACKET_SIZE, SILENCE_PACKET_SIZE, nullBuf, 0
    };

    int offset = 0;

    int waitTime = 0;

    SLAndroidSimpleBufferQueueState state = { 0 };
    (*bqPlayerBufferQueue)->GetState(bqPlayerBufferQueue, &state);

    if (state.count > 1)
    {
        waitTime = (state.count - 1) * 10;
    }

    XStxRawAudioFrame *frame = gAudioRenderer->popFrame(state.count * 10, waitTime);
    if (!frame || frame->mData == NULL || frame->mDataSize == 0)
    {
        // recycle it whether or not it's empty
        if (frame)
        {
            gAudioRenderer->recycleFrame(frame);
        }

        frame = &nullFrame;
        gSkip += nullFrame.mDataSize;
    }
    else
    {
        if (gSkip > 0)
        {
            LOGV("Played %d samples of silence", gSkip / 4);
        }
        gSkip = 0;

        gFrameFreeQueue.push(frame);
    }

    SLresult result;
    // enqueue the next buffer
    result = (*bqPlayerBufferQueue)->Enqueue(bqPlayerBufferQueue, frame->mData, frame->mDataSize);

    if (SL_RESULT_SUCCESS != result)
    {
        LOGE("Failed to Enqueue! %lu", (long unsigned int)result);
    }
    (void)result;
}

uint32_t AudioDownsampleSize(uint32_t frameSize)
{
    return av_rescale_rnd(frameSize, 44100, 48000, AV_ROUND_UP);
}

uint32_t AudioDownsample(XStxRawAudioFrame *frame48k, uint8_t *frame44k)
{
    if (!gSwrContext)
    {
        gSwrContext = swr_alloc();
        if (!gSwrContext)
        {
            LOGE("Could not allocate resampler context");
            assert(false);
            return 0;
        }

        /* set options */
        av_opt_set_int(gSwrContext, "in_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(gSwrContext, "in_sample_rate", 48000, 0);
        av_opt_set_sample_fmt(gSwrContext, "in_sample_fmt", AV_SAMPLE_FMT_S16, 0);
        av_opt_set_int(gSwrContext, "out_channel_layout", AV_CH_LAYOUT_STEREO, 0);
        av_opt_set_int(gSwrContext, "out_sample_rate", 44100, 0);
        av_opt_set_sample_fmt(gSwrContext, "out_sample_fmt", AV_SAMPLE_FMT_S16, 0);

        int ret;
        ret = av_opt_set_int(gSwrContext, "dither_method", SWR_DITHER_TRIANGULAR, 0);

        if ((ret = swr_init(gSwrContext)) < 0)
        {
            LOGE("Failed to initialize gSwrContext (%d)", ret);
            assert(false);
            return 0;
        }
    }
    assert(frame48k && frame44k && frame48k->mData && gSwrContext);

    int sourceSamples = frame48k->mDataSize / (NUM_CHANNELS * AndroidAudioRenderer::BYTES_PER_SAMPLE);
    int destSamples = AudioDownsampleSize(sourceSamples);

    /* Downsample */
    int ret = swr_convert(gSwrContext,
                          &frame44k,
                          destSamples,
                          (const uint8_t **)&frame48k->mData,
                          sourceSamples);

    if (ret < 0)
    {
        LOGE("Failed in calling swr_convert: %d", ret);
        assert(false);
        return 0;
    }

    return destSamples * (NUM_CHANNELS * AndroidAudioRenderer::BYTES_PER_SAMPLE);
}

#define CHECK_RESULT(XX) if ((XX)!=SL_RESULT_SUCCESS) { assert(false); LOGE("Failed when trying to %s",#XX); return ; }

// Create the engine and output mix objects.
//
// See the documentation that comes with the Android NDK for
// more information: docs/opensles/*
//
void OpenSLCreate(AudioRenderer *renderer)
{
    if (gAudioRenderer == renderer)
    {
        return;
    }

    gAudioRenderer = renderer;

    // create engine
    CHECK_RESULT(slCreateEngine(&gEngineObject, 0, NULL, 0, NULL, NULL));

    // realize the engine
    CHECK_RESULT((*gEngineObject)->Realize(gEngineObject, SL_BOOLEAN_FALSE));

    // get the engine interface, which is needed in order to create other objects
    CHECK_RESULT((*gEngineObject)->GetInterface(gEngineObject, SL_IID_ENGINE, &gEngineEngine));

    // create output mix
    CHECK_RESULT((*gEngineEngine)->CreateOutputMix(gEngineEngine, &outputMixObject, 0, NULL, NULL));

    // realize the output mix
    CHECK_RESULT((*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE));

    // configure audio source
    SLDataLocator_AndroidSimpleBufferQueue loc_bufq = { SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, FRAME_COUNT };
    SLDataFormat_PCM format_pcm =
    {
        SL_DATAFORMAT_PCM,
        NUM_CHANNELS,
        SL_SAMPLINGRATE_44_1,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
        SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource audioSrc = { &loc_bufq, &format_pcm };

    // configure audio sink
    SLDataLocator_OutputMix loc_outmix = { SL_DATALOCATOR_OUTPUTMIX, outputMixObject };
    SLDataSink audioSnk = { &loc_outmix, NULL };

    {
        // create audio player
        const SLInterfaceID ids[2] = { SL_IID_BUFFERQUEUE, /*SL_IID_MUTESOLO,*/ SL_IID_VOLUME };
        const SLboolean req[2] = { SL_BOOLEAN_TRUE, /*SL_BOOLEAN_TRUE,*/ SL_BOOLEAN_TRUE };
        CHECK_RESULT((*gEngineEngine)->CreateAudioPlayer(gEngineEngine, &bqPlayerObject, &audioSrc, &audioSnk,
                                                         2, ids, req));
    }

    // realize the player
    CHECK_RESULT((*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE));

    // get the play interface
    CHECK_RESULT((*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay));

    // get the buffer queue interface
    CHECK_RESULT((*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
                                                 &bqPlayerBufferQueue));

    // register callback on the buffer queue
    CHECK_RESULT((*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, bufferQueuePlayerCallback, NULL));

    // get the volume interface
    CHECK_RESULT((*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_VOLUME, &bqPlayerVolume));
}

void OpenSLSetPlaying(JNIEnv *env, jclass clazz, jboolean isPlaying)
{
    LOGV("OpenSLSetPlaying");

    // set the player's state to playing
    SLresult result = (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, isPlaying ? SL_PLAYSTATE_PLAYING : SL_PLAYSTATE_STOPPED);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    if (isPlaying && !gQueuedAudio)
    {
        // queue FRAME_COUNT buffers
        for (int i = 0; i < FRAME_COUNT; ++i)
        {
            bufferQueuePlayerCallback(bqPlayerBufferQueue, NULL);
        }
        gQueuedAudio = true;
    }
}

// shut down the native audio system
void OpenSLShutdown()
{
    LOGV("OpenSLShutdown");

    if (bqPlayerPlay)
    {
        (*bqPlayerPlay)->SetPlayState(bqPlayerPlay, SL_PLAYSTATE_STOPPED);
    }

    gAudioRenderer = NULL;
    mud::ScopeLock scope(gCallbackLock);

    LOGV("OpenSLShutdown inside scope");

    // destroy buffer queue audio player object, and invalidate all associated interfaces
    if (bqPlayerObject != NULL)
    {
        (*bqPlayerObject)->Destroy(bqPlayerObject);
        bqPlayerObject = NULL;
        bqPlayerPlay = NULL;
        bqPlayerBufferQueue = NULL;
        bqPlayerMuteSolo = NULL;
        bqPlayerVolume = NULL;
    }

    LOGV("OpenSLShutdown destroy mix");
    // destroy output mix object, and invalidate all associated interfaces
    if (outputMixObject != NULL)
    {
        (*outputMixObject)->Destroy(outputMixObject);
        outputMixObject = NULL;
    }

    LOGV("OpenSLShutdown destroy engine");
    // destroy engine object, and invalidate all associated interfaces
    if (gEngineObject != NULL)
    {
        (*gEngineObject)->Destroy(gEngineObject);
        gEngineObject = NULL;
        gEngineEngine = NULL;
    }

    LOGV("OpenSLShutdown done");
}

void OpenSLSetMute(bool mute)
{
    if (bqPlayerVolume)
    {
        (*bqPlayerVolume)->SetMute(bqPlayerVolume, mute);
    }
}
