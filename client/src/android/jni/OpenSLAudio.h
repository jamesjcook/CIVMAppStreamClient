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

#ifndef _included_OpenSLAudio_h
#define _included_OpenSLAudio_h

/**
 * @file OpenSLAudio.h
 * OpenSL implementation calls.
 */
#include <jni.h>

// for native audio
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

// for native asset manager
#include <sys/types.h>

#include <MUD/threading/ScopeLock.h>
#include <MUD/threading/SimpleLock.h>

#include "AudioRenderer.h"

class AudioRenderer;

/**
 * Create the engine, output mix objects, and player.
 *
 * @param[in] renderer The renderer to interact with.
 */
void OpenSLCreate(AudioRenderer *renderer);

/**
 * set the playing state for the audio player
 * to PLAYING (true) or PAUSED (false)
 *
 * @param[in] env       Current JNI environment.
 * @param[in] clazz     Java class associated with this function.
 * @param[in] isPlaying True to set playing; false to stop playing.
 */
void OpenSLSetPlaying(JNIEnv *env, jclass clazz, jboolean isPlaying);

/*
 * shut down the native audio system
 */
void OpenSLShutdown();

/*
 * Mute the sound.
 *
 * @param[in] pause  True to mute.
 */
void OpenSLSetMute(bool pause);

/*
 * Calculate the size needed for a downsampled frame based on a given frame size.
 * Downsamples from 48kHz to 44.1kHz.
 *
 * @param[in] frameSize Source frame size.
 *
 * @return Downsampled frame size.
 */
uint32_t AudioDownsampleSize(uint32_t frameSize);
/**
 * Downsample from a 48kHz frame to a 44.1kHz frame.
 *
 * @param[in] frame48k Source frame.
 * @param[in] frame44k Destination frame.
 *
 * @return The destination frame size.
 */
uint32_t AudioDownsample(XStxRawAudioFrame *frame48k, uint8_t *frame44k);

/**
 * Number of channels.
 */
static const uint32_t NUM_CHANNELS = 2;
/**
 * Sampling rate of source audio.
 */
static const uint32_t SAMPLING_RATE = 48000;

/**
 * Number of samples we expect per frame (10ms at 48kHz).
 */
static const uint32_t SAMPLES_PER_FRAME = 480;

/**
 * A MUTEX that blocks the adding of audio frames.
 */
extern mud::SimpleLock gCallbackLock;

#endif //_included_OpenSLAudio_h
