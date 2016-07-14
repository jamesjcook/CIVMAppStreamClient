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


/**
 * CoreAudioRenderer implementation
 */

#include "CoreAudioRenderer.h"
#include "MUD/threading/ScopeLock.h"
#include "MUD/base/TimeVal.h"
#include "log.h"

static const int OUTPUT_BUS = 0;                // the default output speaker
static const int NUM_CHANNELS  = 2;             // as per XStxClientAPI
static const int SAMPLERATE = 48000;            // in khz
static const int SUGGESTED_CA_LATENCY_MS = 8;   // in milliseconds.
static const int NUM_BLOCKS_TO_BUFFER_CA = 6;   //
static const int NUM_BLOCKS_DROP_THRESHOLD = 12;
static const int BYTES_PER_SAMPLE = 2;
static const uint32_t US_PER_SAMPLE = 10000;
static double startTime;

static AudioStreamBasicDescription audioStreamFormat;

/**
 Core Audio Renderer
 */

CoreAudioRenderer::CoreAudioRenderer(
                                     mud::FixedSizePool<XStxRawAudioFrame> & framePool,
                                     XStxClientHandle clientHandle)
:
AudioRenderer(framePool,clientHandle),
mFramePool(framePool),
mAudioIsPlaying(false),
mFrame(NULL),
mPlaybackPosInFrame(0),
mDidInit(false),
mClientHandle(clientHandle)
{
    lastTimeStamp = 0;
}

CoreAudioRenderer::~CoreAudioRenderer()
{
    XThrowIfError(AudioOutputUnitStop(this->mAudioUnit), "couldn't stop audio unit");
}



/**
 This callback is invoked by the output audio unit when it needs more
 audio data to render.
 */
static OSStatus audioUnitRenderCallback(void *context,
                                        AudioUnitRenderActionFlags *ioActionFlags,
                                        const AudioTimeStamp *inTimeStamp,
                                        UInt32 inBusNumber,
                                        UInt32 inNumberFrames,
                                        AudioBufferList *outputData) {
    // hand off to audio renderer
    CoreAudioRenderer *ctx = (CoreAudioRenderer*) context;
    if(ctx != NULL){
        ctx->fillOutputAudioBufferList(inNumberFrames, outputData,inTimeStamp, ioActionFlags);
    }
    
    return noErr;
}


XStxResult CoreAudioRenderer::start()
{
    if (!mDidInit)
    {
        if (!initialize())
        {
            return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
        }
        mDidInit = true;
    } else if (!mAudioIsPlaying)
    {
        //If it's already playing we still need to start the audio unit
        if (!startAudioUnit()) {
            return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
        }
    }
    return XSTX_RESULT_OK;
}

/**
 * fill the output audio buffer list with audio data
 */

bool CoreAudioRenderer::fillOutputAudioBufferList( UInt32 numberOfFrames,
                                                  AudioBufferList *outputData,
                                                  const AudioTimeStamp *timeStamp,
                                                  AudioUnitRenderActionFlags *ioActionFlags )
{
    UInt32 bytesPerFrame        = audioStreamFormat.mBytesPerFrame;
    UInt32 numBytesRequested    = numberOfFrames * bytesPerFrame;
    
    // in simple playback configuration (one output)
    // this is called once for each buffer
    if(outputData->mNumberBuffers==0) return false;
    
    AudioBuffer buffer = outputData->mBuffers[0];
    
    // clear contents of buffer
    memset((Byte*)buffer.mData,0,buffer.mDataByteSize);
    
    // reset buffer fields
    UInt32 numBytesWritten = 0;
    buffer.mDataByteSize  = 0;
    
    // params for the GetNextAudioFrame method
    uint32_t  maxWaitTime = 0;
    uint32_t timeout = 0;
    
    // loop through frames until buffer has enough data
    while( numBytesWritten < numBytesRequested )
    {
        
        if(mPlaybackPosInFrame > 0 && numBytesWritten ==0)
        {
            // use the existing mFrame because it still contains data,
            // starting at mPlaybackPosInFrame
        }
        else // get a new frame
        {
            XStxResult getframeResult = XSTX_RESULT_OK;
            
            mFrame = popFrame(maxWaitTime, timeout);
            
            if( mFrame == NULL)
            {
                getframeResult = XSTX_RESULT_FRAME_NOT_AVAILABLE;
                
            }
            
            if (getframeResult != XSTX_RESULT_OK)
            {
//                LOGE("Filling audio buffer with some silence");
                // fill requested buffer with requested size of silence
                buffer.mDataByteSize = numBytesRequested;
                memset((Byte*)buffer.mData,0,buffer.mDataByteSize);
                *ioActionFlags |=  kAudioUnitRenderAction_OutputIsSilence;
                break;
            }
        }
        
        uint32_t maxBytesInFrame = (mFrame->mDataSize - mPlaybackPosInFrame);
        uint32_t maxBytesCanWrite = numBytesRequested - numBytesWritten;
        uint32_t numBytesToTake = maxBytesInFrame > maxBytesCanWrite ?
        maxBytesCanWrite : maxBytesInFrame;
        
        memcpy((uint8_t*)buffer.mData+numBytesWritten, mFrame->mData+mPlaybackPosInFrame, numBytesToTake);
        
        mPlaybackPosInFrame += numBytesToTake;
        numBytesWritten += numBytesToTake;
        
        if(mPlaybackPosInFrame >= mFrame->mDataSize)
        {
            mPlaybackPosInFrame = 0;
            mFramePool.recycleElement(mFrame);
            mFrame=NULL;
        }
        
    }
    
    buffer.mDataByteSize = numBytesWritten;
    
    return true;
}


// initialize core audio units / audio session setup
bool CoreAudioRenderer::initialize(){
    startTime = 0.0f;
    OSStatus status;
    
    if( mDidInit == true)
    {
        return true;
    }
    
    
    // Describe audio component to load
    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentFlags = 0;
    desc.componentFlagsMask = 0;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    
    // Get output device based on the description
    AudioComponent outputComponent = AudioComponentFindNext(NULL, &desc);
    XThrowIf(outputComponent==NULL, kAudioServicesNoHardwareError,
             "Could not get handle to hardware device");
    
    // Get handle to new audio unit instance
    XThrowIfError(AudioComponentInstanceNew(outputComponent, &this->mAudioUnit),
                  "could not create new audio unit instance");
    
    // do not create a buffer to render into upon output
    UInt32 flag = 0;
    XThrowIfError(AudioUnitSetProperty(mAudioUnit,
                                       kAudioUnitProperty_ShouldAllocateBuffer,
                                       kAudioUnitScope_Output,
                                       OUTPUT_BUS,
                                       &flag,
                                       sizeof(flag)),"could not enable audio output");
    
    // Describe stream format of incoming decoded pcm audio
    audioStreamFormat.mSampleRate           = SAMPLERATE;
    audioStreamFormat.mFormatID             = kAudioFormatLinearPCM;
    audioStreamFormat.mFormatFlags          = kAudioFormatFlagIsSignedInteger | kAudioFormatFlagIsPacked;
    audioStreamFormat.mFramesPerPacket      = 1;
    audioStreamFormat.mChannelsPerFrame     = 2;
    audioStreamFormat.mBitsPerChannel       = 8 * sizeof (SInt16);
    audioStreamFormat.mBytesPerPacket       = 2 * sizeof (SInt16);
    audioStreamFormat.mBytesPerFrame        = 2 * sizeof (SInt16);
    
    // set format on the audio unit to stream format
    XThrowIfError(AudioUnitSetProperty(mAudioUnit,
                                       kAudioUnitProperty_StreamFormat,
                                       kAudioUnitScope_Input,
                                       OUTPUT_BUS,
                                       &audioStreamFormat,
                                       sizeof(audioStreamFormat)),
                  "could not set output stream on audio unit");
    
    // called when audio unit needs more audio data to render
    AURenderCallbackStruct callbackStruct;
    callbackStruct.inputProc = audioUnitRenderCallback;
    
    //set reference to "this" which becomes *context in the AudioUnitRenderCallback
    callbackStruct.inputProcRefCon = this;
    
    XThrowIfError(AudioUnitSetProperty(mAudioUnit,
                                       kAudioUnitProperty_SetRenderCallback,
                                       kAudioUnitScope_Global,
                                       OUTPUT_BUS,
                                       &callbackStruct,
                                       sizeof(callbackStruct)),
                  "could not set audio unit rendering callback");
    
    XThrowIfError(AudioUnitInitialize( mAudioUnit ),"could not init core audio unit");
    
    return startAudioUnit();
}

bool CoreAudioRenderer::startAudioUnit()
{
    OSStatus status = AudioOutputUnitStart(mAudioUnit);
    if (status != noErr)
    {
        return false;
    }
    
    return true;
}

XStxRawAudioFrame* CoreAudioRenderer::popFrame(int delay, int msBuffer)
{
    XStxRawAudioFrame * frame = NULL;
    
    XStxResult result = XStxGetNextAudioFrame(mClientHandle, &frame, delay, msBuffer) ;
    if (result!=XSTX_RESULT_OK)
    {
//        LOGV("%s Failed to get audio frame! %d",__PRETTY_FUNCTION__, result);
        if (result==XSTX_RESULT_INVALID_STATE)
        {
            LOGV("%s: %d Invalid state",__PRETTY_FUNCTION__, result);
        }
        return NULL;
    }
    return frame;
}



void CoreAudioRenderer::pause( bool pause )
{
    LOGV("CoreAudioRenderer::pause");
    mPaused = pause;
}

void CoreAudioRenderer::stop()
{
    XThrowIfError(AudioOutputUnitStop(this->mAudioUnit), "couldn't stop audio unit");
    
    if (mFrame) {
        mPlaybackPosInFrame = 0;
        mFramePool.recycleElement(mFrame);
        mFrame = NULL;
    }
}




