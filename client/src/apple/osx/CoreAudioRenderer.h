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


#ifndef _included_CoreAudioRenderer_h
#define _included_CoreAudioRenderer_h

#include <queue>
#include <stdint.h>


#include "MUD/threading/SimpleLock.h"
#include "MUD/memory/FixedSizePool.h"

#include "XStx/common/XStxAPI.h"
#include "XStx/client/XStxClientAPI.h"

#import <AudioToolbox/AudioToolbox.h>
#import <CoreAudio/CoreAudioTypes.h>

#include "AudioRenderer.h"

// for more informative debugging get the CoreAudio Public Utility classes
// https://developer.apple.com/library/mac/samplecode/CoreAudioUtilityClasses/
// and uncomment the folllowing line:
// #define __HAVE_CAEXCEPTION_H__ 1

#ifdef __HAVE_CAEXCEPTION_H__
    #include "CAXException.h"
#else
// simple error handling
#define XThrowIfError(error, op)   \
do { OSStatus __oserr = error; if (__oserr) {  fprintf(stderr,"CoreAudio Error: %s\n",op); assert(false); }} while (0)

#define XThrowIf(condition, error, op)								\
do { if (condition) { fprintf(stderr,"%s\n",op); assert(false) ;}} while (0)

#endif

class CoreAudioRenderer : public AudioRenderer
{

public:
    CoreAudioRenderer(mud::FixedSizePool<XStxRawAudioFrame> & framePool,
                  XStxClientHandle clientHandle);
    ~CoreAudioRenderer();

    bool initialize();

    XStxResult setNumChannels(uint32_t numChannels)
    {
        mNumChannels = numChannels;
        return XSTX_RESULT_OK;
    }
    XStxResult setSamplingRate(uint32_t samplingRate)
    {
        mSamplingRate = samplingRate;
        return XSTX_RESULT_OK;
    }

    XStxResult start();
    void pause( bool pause );
    void stop();

    /**
     * Acquire an audio frame to render.
     *
     * @param[in] msBuffer How much time we can wait for the next frame of
     *                 audio to arrive.
     *
     * @return The frame to render, or NULL if none is available.
     */
    XStxRawAudioFrame* popFrame(int delay, int msBuffer);

    XStxResult renderFrame(XStxRawAudioFrame * frame);

    bool fillOutputAudioBufferList( UInt32 numberOfFrames,
                                      AudioBufferList *outputData,
                                      const AudioTimeStamp *timeStamp,
                                      AudioUnitRenderActionFlags *ioActionFlags );

    // core audio toolbox
    AudioComponentInstance mAudioUnit;
    AUGraph mAudioGraph;
    XStxClientHandle mClientHandle;

private:

    void removeLateFrames();
    void reportIntervalsAndGaps(uint32_t numRequested, uint32_t numWritten);
    void reportFrameMetrics();
    bool startAudioUnit();


    uint64_t         lastTimeStamp;
    bool mDidInit;
    bool mPaused;

    // configuration
    uint32_t mNumChannels;
    uint32_t mSamplingRate;

    // buffering and playback
    bool mAudioIsPlaying;
    std::queue<XStxRawAudioFrame*> mFrameQueue;
    mud::FixedSizePool<XStxRawAudioFrame> & mFramePool;
    XStxRawAudioFrame *mFrame;
    uint32_t mPlaybackPosInFrame;

    // metric gathering
    uint64_t mExpectedTimestampUs;
    uint64_t mTimelineIntervalStartUs;
    uint64_t mTimelineGapGraceUs;
    bool mPlaying;
    uint64_t mPlaybackInterval;
    uint64_t mPlaybackGap;

    // configuration
    uint32_t mUsPerSample;



};

#endif

