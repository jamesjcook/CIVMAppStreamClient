/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#define FRAME_POOL_SIZE 10  // frame buffer size
#define SHUTDOWN_TIMEOUT_COUNT 100 // 100 times of 100 milliseconds = 10-seconds timeout for game shutdown
#define SHUTDOWN_TIMEOUT_PERIOD 100   // 100 milliseconds increment
#include <string>
#include <unordered_set>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "HostedApplication.h"

#include "XStx/common/XStxUtil.h"
#include "XStx/common/XStxResultAPI.h"

#include "Color/ColorConversions.h"

#include "Game.h"   // DirectX example

/**
 * HostedApplication is the interface presented by a simple application
 * driven by SessionManagerListener from the XStxExampleServer example.
 *
 * The static 'Imp' derivation is used to hide implementation
 * details from consumers of HostedApplication.  In particular, this
 * allows the use of HostedApplication by the example code while
 * allowing different implementations (FogStreamingLauncher and
 * XStxExampleServer).
 *
 */
class HostedApplicationImp
    :
    public HostedApplication,
    private XStxIServerListener2,
    private XStxIInputSink,
    private XStxIAudioSource,
    private XStxIVideoSource
{
public:

    HostedApplicationImp(
        XStxServerHandle server,
        const char* context)
        : mContext(context)
        , mServer(server)
        , mVideoWidth(1280)
        , mVideoHeight(720)
        , mGame(NULL)
        , mChromaSampling(XSTX_CHROMA_SAMPLING_UNKNOWN)
    {
        /** Initialize the various XStx interfaces */

        XSTX_INIT_INTERFACE_SIZE(XStxIServerListener2)
        XSTX_INIT_CALLBACK(XStxIServerListener2, Ready)
        XSTX_INIT_CALLBACK(XStxIServerListener2, Reconnecting)
        XSTX_INIT_CALLBACK(XStxIServerListener2, Reconnected)
        XSTX_INIT_CALLBACK(XStxIServerListener2, Stopped)
        XSTX_INIT_CALLBACK(XStxIServerListener2, MessageReceived)
        XSTX_INIT_CALLBACK(XStxIServerListener2, SetConfiguration)

        XSTX_INIT_INTERFACE_SIZE(XStxIInputSink)
        XSTX_INIT_CALLBACK(XStxIInputSink, OnInput)

        XSTX_INIT_INTERFACE_SIZE(XStxIVideoSource)
        XSTX_INIT_CALLBACK(XStxIVideoSource, GetMode)
        XSTX_INIT_CALLBACK(XStxIVideoSource, SetFrameRate)
        XSTX_INIT_CALLBACK(XStxIVideoSource, Start)
        XSTX_INIT_CALLBACK(XStxIVideoSource, GetFrame)
        XSTX_INIT_CALLBACK(XStxIVideoSource, RecycleFrame)
        XSTX_INIT_CALLBACK(XStxIVideoSource, Stop)

        /** Parse the app context. For this sample application the app context
         * is assumed to be a string of the form key1=value1&key2=value2 ...
         */
        parseAppContext();

        // notify STX server that we can handle YUV420 format
        if (XSTX_RESULT_OK == XStxServerAddChromaSamplingOption(
                mServer, XSTX_CHROMA_SAMPLING_YUV420))
        {
            printf("Successfully notified of YUV420 capability\n");
        }
        else
        {
            printf("Failed to register YUV420 capability\n");
        }

        /**===============================================================
         * YUV444 option
         *
         * Uncomment the following block to enable YUV444 streaming option
         */

        /*
        // notify STX server that we can handle YUV444 format
        if (XSTX_RESULT_OK == XStxServerAddChromaSamplingOption(
                mServer, XSTX_CHROMA_SAMPLING_YUV444))
        {
            printf("Successfully notified of YUV444 capability\n");
        }
        else
        {
            printf("Failed to register YUV444 capability\n");
        }
        */

        /**
         * ===============================================================
         */

        InitializeCriticalSection(&m_frameCritSec);

        printf("Creating hosted app: width = %d, height = %d\n", mVideoWidth, mVideoHeight);
        // instantiate game and initialize game resources
        mGame = Game::startGame(this, "AppStream Example Game", "WWExampleGameWndClass",
                                mVideoWidth, mVideoHeight);
        // wait till the initialization phase completes
        while( !mGame->getInitialized() && !mGame->isError() )
        {
            Sleep(10); // sleep 10 milliseconds
        }
        if (mGame->isError())
        {
            // error in initialization
            printf("There was error initializing game...\n");
        }
    }

    /** reads integer value from key-value pair */
    bool intValFromKey(uint32_t& val, const std::string key, const std::string src)
    {
        std::size_t startPos;
        std::size_t endPos;
        if((startPos = src.find(key)) != std::string::npos)
        {
            startPos += key.length();
            endPos = src.find_first_of('&', startPos);
            val = atoi(std::string(src, startPos, endPos).c_str());
            return true;
        }
        return false;
    }

    /** parse command-line arguments */
    void parseAppContext()
    {
        std::string clean = mContext;
        size_t quote ;
        while ( (quote=clean.find('"'))!=std::string::npos )
        {
            clean.erase(quote,1);
        }        //Starting string with & and ending with & so all keys begin with &
        //and all values end with &. This makes it easier to distinguish keys 
        //and values.
        std::string context("&" + clean + "&");

        uint32_t val;
        if(intValFromKey(val, "&width=", context))
        {
            mVideoWidth = val;
        }
        if(intValFromKey(val, "&height=", context))
        {
            mVideoHeight = val;
        }
    }

    /** destructor */
    ~HostedApplicationImp()
    {
        if (NULL != mGame)
        {
            delete mGame;
            mGame = NULL;
        }
        DeleteCriticalSection(&m_frameCritSec);
    }

    /** set the server */
    XStxResult setServer(XStxServerHandle server)
    {
        mServer = server;
        return XSTX_RESULT_OK;
    }

    XStxResult start()
    {
        return XSTX_RESULT_OK;
    }

    /** I am server listener myself */
    XStxIServerListener2* getServerListener()
    {
        return this;
    }

    /** I am input sink myself */
    XStxIInputSink* getInputSink()
    {
        return this;
    }

    /** I am audio source myself */
    XStxIAudioSource* getAudioSource()
    {
        return this;
    }

    /** I am video source myself */
    XStxIVideoSource* getVideoSource()
    {
        return this;
    }

    /** allocate a video frame */
    XStxRawVideoFrame* allocateVideoFrame(XStxChromaSampling chromaSampling);

    /** de-allocate a video frame */
    void deallocateVideoFrame(XStxRawVideoFrame* frame);

    // Helper functions to get and put frames from/to the pool of raw frames
    XStxRawVideoFrame* HostedApplicationImp::takeFrameFromPool()
    {
        XStxRawVideoFrame* frame = NULL;
        EnterCriticalSection(&m_frameCritSec);
        if ( !mVideoFrames.empty())
        {
            // fetch a frame from pool and populate it
            auto it = mVideoFrames.begin();
            frame = *it;
            mVideoFrames.erase(it);
        }
        LeaveCriticalSection(&m_frameCritSec);

        return frame;
    }

    void HostedApplicationImp::putFrameInPool(XStxRawVideoFrame* frame)
    {
        EnterCriticalSection(&m_frameCritSec);
        mVideoFrames.insert(frame);
        LeaveCriticalSection(&m_frameCritSec);
    }

    /**
     * The game wants to post a new RGBA/BGRA/YUV frame.. convert & save it
     * so it can be returned when Stx asks for a new frame
     */
    void HostedApplicationImp::postNewFrame(const unsigned char* theFrame, CapturePixelFormat pixelformat)
    {
        XStxRawVideoFrame* frame = takeFrameFromPool();
        if (NULL == frame)
            return;
        bool convertResult = true;
        int hWidth = mVideoWidth >> 1;
        int hHeight = mVideoHeight >> 1;
        bool const yuv444 = mChromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
        switch (pixelformat)
        {
            case CapturePixelFormat::CAPTURE_PIXELFORMAT_YUV420:
                {
                    // Already int the correct format, just copy the planes
                    const unsigned char* theFramePtr = theFrame;
                    CopyMemory(frame->mPlanes[0], theFramePtr, mVideoWidth * mVideoHeight);
                    theFramePtr += mVideoWidth * mVideoHeight;
                    CopyMemory(frame->mPlanes[1], theFramePtr, hWidth * hHeight);
                    theFramePtr += hWidth * hHeight;
                    CopyMemory(frame->mPlanes[2], theFramePtr, hWidth * hHeight);
                }
                break;

            case CAPTURE_PIXELFORMAT_B8G8R8:
                {
                    convertResult = yuv444
                        ? convertToYUV444(theFrame, mVideoWidth, mVideoHeight, 2, 1, 0, 3, mVideoWidth * 3, frame->mPlanes)
                        : convertToYUV420(theFrame, mVideoWidth, mVideoHeight, 2, 1, 0, 3, mVideoWidth * 3, frame->mPlanes);
                }
                break;
            case CAPTURE_PIXELFORMAT_B8G8R8A8:
                {
                    convertResult = yuv444
                        ? convertToYUV444(theFrame, mVideoWidth, mVideoHeight, 2, 1, 0, 4, mVideoWidth * 4, frame->mPlanes)
                        : convertToYUV420(theFrame, mVideoWidth, mVideoHeight, 2, 1, 0, 4, mVideoWidth * 4, frame->mPlanes);
                }
                break;
            case CAPTURE_PIXELFORMAT_R8G8B8A8:
                {
                    convertResult = yuv444
                        ? convertToYUV444(theFrame, mVideoWidth, mVideoHeight, 0, 1, 2, 4, mVideoWidth * 4, frame->mPlanes)
                        : convertToYUV420(theFrame, mVideoWidth, mVideoHeight, 0, 1, 2, 4, mVideoWidth * 4, frame->mPlanes);
                }
                break;
        }
        if (convertResult)
            XStxServerPushVideoFrame(mServer, frame); // Push the frame for delivery
        else
            putFrameInPool(frame); // Conversion failed, put the frame back
    }

private:

    /**
     * Create static methods satisfying the various XStx interfaces and declare
     * the corresponding non-static methods.
     */

    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIServerListener2Ready)
    XSTX_DECLARE_CALLBACK_4(HostedApplicationImp, XStxIServerListener2Reconnecting, uint32_t, XStxDisconnectReason, uint32_t, const unsigned char*)
    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIServerListener2Reconnected)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIServerListener2Stopped, XStxStopReason)
    XSTX_DECLARE_CALLBACK_2(HostedApplicationImp, XStxIServerListener2MessageReceived, const unsigned char*, uint32_t)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIServerListener2SetConfiguration, const XStxServerConfiguration*)

    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIInputSinkOnInput, const XStxInputEvent*)

    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIAudioSourceGetNumberOfChannels, uint32_t*)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIAudioSourceGetSamplesPerSecond, uint32_t*)
    XSTX_DECLARE_CALLBACK_2(HostedApplicationImp, XStxIAudioSourceGetBytesPerSample, uint32_t*, XStxBool*)
    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIAudioSourceStart)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIAudioSourceGetFrame, XStxRawAudioFrame**)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIAudioSourceRecycleFrame, XStxRawAudioFrame*)
    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIAudioSourceStop)

    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIVideoSourceGetMode, XStxVideoMode*)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIVideoSourceSetFrameRate, double)
    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIVideoSourceStart)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIVideoSourceGetFrame, XStxRawVideoFrame**)
    XSTX_DECLARE_CALLBACK_1(HostedApplicationImp, XStxIVideoSourceRecycleFrame, XStxRawVideoFrame*)
    XSTX_DECLARE_CALLBACK_0(HostedApplicationImp, XStxIVideoSourceStop)

    /** instance data */
    std::string mContext;
    XStxServerHandle mServer;

    /** video frame pool */
    std::unordered_set< XStxRawVideoFrame* > mVideoFrames;


    /** 
     * Video Resolution for initializing the video generator. These are set to
     * 1280 by 720 by default. If the app context string contains keys for setting 
     * the resolution, these get set according to the app context. 
     */
    uint32_t mVideoWidth;
    uint32_t mVideoHeight;
    XStxChromaSampling mChromaSampling;

    DWORD theGameThread;
    Game * mGame;

    /** for locking mutex */
    CRITICAL_SECTION m_frameCritSec;
};


XStxResult HostedApplicationImp::XStxIServerListener2Ready()
{
    printf("[HostedApplication] ServerListenerServerReady called...\n");
    if (NULL == mServer || NULL == mGame || !mGame->getInitialized())
    {
        return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
    }
    return XSTX_RESULT_OK;
}


XStxResult HostedApplicationImp::XStxIServerListener2Reconnecting(
                                                        uint32_t timeoutMs,
                                                        XStxDisconnectReason reason,
                                                        uint32_t hintSize,
                                                        const unsigned char* hintData)
{
    printf("Reconnecting: %u\n", timeoutMs);
    return XSTX_RESULT_OK;
}

XStxResult HostedApplicationImp::XStxIServerListener2Reconnected()
{
    printf("Reconnected\n");
    return XSTX_RESULT_OK;
}

/** server stop called */
XStxResult HostedApplicationImp::XStxIServerListener2Stopped(
    XStxStopReason reason)
{
    printf("[HostedApplication] ServerListenerServerStopped called...\n");
    UNREFERENCED_PARAMETER(reason);
    return XSTX_RESULT_OK;
}

/** listener've got mail ! */
XStxResult HostedApplicationImp::XStxIServerListener2MessageReceived(
    const unsigned char* message, uint32_t size)
{
    // TODO: handle message
    std::string msg((const char*) message, size);
    printf("Got message: %s\n", msg.c_str());

    return XSTX_RESULT_OK;
}

XStxResult HostedApplicationImp::XStxIServerListener2SetConfiguration(
    const XStxServerConfiguration* config)
{
    // received server configuration
    printf("Received server configuration...\n");
    
    mChromaSampling = config->mChromaSampling;
    if (mChromaSampling == XSTX_CHROMA_SAMPLING_YUV420)
    {
        printf("[Server Configuration] using YUV420\n");
    }
    else if (mChromaSampling == XSTX_CHROMA_SAMPLING_YUV444)
    {
        printf("[Server Configuration] using YUV444\n");
    }
    else
    {
        printf("[Server Configuration] unkonwn chroma sampling option\n");
    }
    return XSTX_RESULT_OK;
}

/**
 * Handling keyboard + mouse input
 * F12 quits the server. All other keyboard inputs are delegated to the game.
 * Mouse input is ignored
 */
XStxResult HostedApplicationImp::XStxIInputSinkOnInput(const XStxInputEvent* event)
{
    if (NULL == event)
        return XSTX_RESULT_INVALID_ARGUMENTS;

    if ((event->mType == XSTX_INPUT_EVENT_TYPE_KEYBOARD) &&
        (event->mInfo.mKeyboard.mIsKeyDown) &&
        (event->mInfo.mKeyboard.mVirtualKey == 78))
    {
        printf("Received N-key\n");
        std::string msg("Server received N-keyboard input!");
        if (XSTX_RESULT_OK == XStxServerSendMessage(
            mServer, (const unsigned char*) msg.c_str(), (uint32_t)msg.size()))
            printf("Successfully sent message to client\n");
        else
            printf("Failed to send message to client\n");
    }
    else
        if (NULL != mGame)
        {
            mGame->handleInput(event); // All other input is delegated to the game
        }
    return XSTX_RESULT_OK;
}

/**
 * Return the video frame type
 * @param[out] videoFrameType enum representing video frame type
 */
XStxResult HostedApplicationImp::XStxIVideoSourceGetMode(XStxVideoMode* mode)
{
    *mode = XSTX_VIDEO_MODE_PUSH_IMMEDIATE;
    return XSTX_RESULT_OK;
}

/** Set the video frame rate */
XStxResult HostedApplicationImp::XStxIVideoSourceSetFrameRate(double rate)
{
    if (NULL != mGame)
    {
        mGame->setMaxFrameRate((float) rate);
    }
    return XSTX_RESULT_OK;
}

/** start the video source generator */
XStxResult HostedApplicationImp::XStxIVideoSourceStart()
{
    printf("[HostedApplication] VideoSourceStart called...\n");
    if (NULL == mServer || !mGame->getInitialized() || mGame->isError())
    {
        printf("[HostedApplication] Server is not ready...\n");
        return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
    }

    // initialize game resources
    if (mChromaSampling == XSTX_CHROMA_SAMPLING_UNKNOWN)
    {
        printf("[HostedApplication] didn't received chroma samling configuration\n");
        return XSTX_RESULT_INVALID_STATE;
    }
    EnterCriticalSection(&m_frameCritSec);
    // allocate video frame pool
    while(mVideoFrames.size() < FRAME_POOL_SIZE)
    {
        XStxRawVideoFrame* newFrame = allocateVideoFrame(mChromaSampling);
        mVideoFrames.insert( newFrame );
    }
    LeaveCriticalSection(&m_frameCritSec);

    // Kick off the game thread and put a wrapper
    printf("[HostedApplication] Starting game...\n");
    mGame->startRendering();

    return XSTX_RESULT_OK;
}

/**
 * Fetch video frame
 * @param[out] xstxFrame pointer to XStxRawVideoFrame pointer. Will be populated with video frame
 */
XStxResult HostedApplicationImp::XStxIVideoSourceGetFrame(XStxRawVideoFrame** xstxFrame)
{
    *xstxFrame = NULL;
    return XSTX_RESULT_VIDEO_FAILED_ALLOCATE_FRAME;
}

/**
 * Recycle video frame
 * @param[in] xstxFrame video frame to be recycled
 */
XStxResult HostedApplicationImp::XStxIVideoSourceRecycleFrame(XStxRawVideoFrame* xstxFrame)
{
    if (NULL == xstxFrame)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    // put it back into frame pool
    EnterCriticalSection(&m_frameCritSec);
    mVideoFrames.insert( xstxFrame );
    LeaveCriticalSection(&m_frameCritSec);

    return XSTX_RESULT_OK;
}

/** stop the video source generator */
XStxResult HostedApplicationImp::XStxIVideoSourceStop()
{
    XStxResult result = XSTX_RESULT_OK;
    printf("[HostedApplication] IVideoSourceStop called.\n");
    // stop the game thread
    if (NULL != mGame)
    {
        mGame->shutdownGame();
    } else {
        printf("Game is already destroyed...\n");
        return XSTX_RESULT_OK;
    }
    printf("[HostedApplication] Waiting for game thread to finish\n");
    int tryCount = 0;
    while(tryCount < SHUTDOWN_TIMEOUT_COUNT && mGame->getRendering())
    {
        ++tryCount;
        Sleep(static_cast<DWORD>(SHUTDOWN_TIMEOUT_PERIOD));  // sleep for 100-milliseconds
    }
    if (mGame->getRendering())
    {
        // 10-seconds timeout
        fprintf(stderr,"[ERROR] Timeout; Game is still rendering onto resources...\n");
        printf("[SEVERE] Application mal-functioning. This should not happen\n");
        result = XSTX_RESULT_TIMED_OUT;
    } else {
        printf("[HostedApplication] Game thread exited\n");
    }

    printf("[HostedApplication] Cleaning up video frame buffers...\n");
    EnterCriticalSection(&m_frameCritSec);
    while(!mVideoFrames.empty())
    {
        auto it = mVideoFrames.begin();
        XStxRawVideoFrame* frame = *it;
        mVideoFrames.erase(it);
        deallocateVideoFrame( frame );
    }
    LeaveCriticalSection(&m_frameCritSec);

    return result;
}

/** initiate the hosted application */
XStxResult HostedApplication::createHostedApplication(
    XStxServerHandle server,
    const char* context,
    HostedApplication*& hostedApplication)
{
    hostedApplication = new HostedApplicationImp(server, context);

    if (NULL == hostedApplication)
    {
        return XSTX_RESULT_OUT_OF_MEMORY;
    }

    return XSTX_RESULT_OK;
}

/**
 * allocate a video frame
 * @return a newly allocated XStxRawVideoFrame structure
 */
XStxRawVideoFrame* HostedApplicationImp::allocateVideoFrame(
    const XStxChromaSampling chromaSampling)
{
    if (chromaSampling != XSTX_CHROMA_SAMPLING_YUV420 &&
        chromaSampling != XSTX_CHROMA_SAMPLING_YUV444)
    {
        printf("[ERROR] Unknown chroma sub-sampling option given: %d\n", static_cast<int>(chromaSampling));
        assert(0);
        return NULL;
    }
    XStxRawVideoFrame * frame = new XStxRawVideoFrame;
    frame->mHeight = mVideoHeight;
    frame->mWidth = mVideoWidth;
    frame->mSize = sizeof(XStxRawVideoFrame);
    frame->mTimestampUs = 0;

    bool const yuv444 = chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;

    int const areaY    = mVideoWidth * mVideoHeight;
    int const strideY  = mVideoWidth;
    int const areaUV   = yuv444 ? areaY : areaY / 4;
    int const strideUV = yuv444 ? strideY : strideY / 2;

    for (int j=0 ; j<3 ; j++)
    {
        if (j==0)
        {
            frame->mPlanes[ j ] = new unsigned char[areaY];
            frame->mStrides[ j ] = strideY;
            frame->mBufferSizes[ j ] = areaY;
        } else {
            frame->mPlanes[ j ] = new unsigned char[areaUV];
            frame->mStrides[ j ] = strideUV;
            frame->mBufferSizes[ j ] = areaUV;
        }
    }
    return frame;
}

/**
 * de-allocate video frame
 * @param[in] frame pointer to XStxRawVideoFrame structure
 */
void HostedApplicationImp::deallocateVideoFrame(XStxRawVideoFrame* frame)
{
    if (frame != NULL)
    {
        delete frame->mPlanes[0];
        delete frame->mPlanes[1];
        delete frame->mPlanes[2];
        delete frame;
        frame = NULL;
    }
}
