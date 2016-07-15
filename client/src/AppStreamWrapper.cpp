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


#include "AppStreamWrapper.h"
#include "AudioRenderer.h"

#include "VideoPipeline.h"

#undef LOG_TAG
#define LOG_TAG "AppStreamWrapper"
#include "log.h"

#include <MUD/base/AStdio.h>
#include <MUD/threading/SimpleLock.h>
#include <MUD/threading/ScopeLock.h>

AppStreamWrapper::AppStreamWrapper() :
    mVideoRenderer(NULL),
    mClientHandle(NULL),
    mClientLibraryHandle(NULL),
    mFrameCount(0),
    mLastFrameRateStart(0),
    mLastTotalCount(0),
    mTimePassedInSeconds(0),
    mPaused(false),
    mReconnecting(false),
#if APPSTREAM_ENABLE_METRICS
    mMetricsCount(0),
    mLastMetricsCount(0),
#endif
    mStreamingState(-1)
{
#if APPSTREAM_ENABLE_METRICS
    // Metrics calculations. Useful for diagnosing issues, but
    // you can ignore them while you're setting up a new AppStream
    // app.
    memset(&mMetricsAccum,0,sizeof(XStxStreamQualityMetrics));
    memset(&mLastMetrics,0,sizeof(XStxStreamQualityMetrics));
#endif
}

AppStreamWrapper::~AppStreamWrapper()
{
    recycle();
    delete mVideoRenderer;
}

// Forward declarations of callbacks.
XStxResult clientReady(
                void *context,
                uint32_t size,
                const uint8_t* buffer);

XStxResult clientReconnecting(
                    void *context,
                    uint32_t timeoutMs,
                    XStxDisconnectReason reason);

XStxResult clientReconnected(void *context);

XStxResult clientStopped(void *context,  XStxStopReason stopReason);

XStxResult clientQoS(void *context, const XStxStreamQualityMetrics* metrics);

XStxResult receivedClientConfiguration(void *context, const XStxClientConfiguration* config);

XStxResult messageReceived(
                    void *context,
                    const unsigned char *message,
                    uint32_t length);

XStxResult AppStreamWrapper::init()
{
    XStxResult createResult = XSTX_RESULT_OK;
    if ((createResult = XStxClientLibraryCreate(XSTX_CLIENT_API_VERSION_MAJOR,
                                                XSTX_CLIENT_API_VERSION_MINOR, &mClientLibraryHandle)) != XSTX_RESULT_OK)
    {
        const char *name; const char *desc;
        XStxResultGetInfo(createResult, &name, &desc);
        LOGE("XStxClientLibraryCreate failed with: %s", name);
        return createResult;
    }

    mVideoRenderer = newVideoRenderer();

    return XSTX_RESULT_OK;
}


XStxResult AppStreamWrapper::connect(std::string address)
{
	LOGV("connect(%s)", address.c_str());
	memset(&mStxListener, 0, sizeof(mStxListener));

	if (mClientHandle != NULL)
	{
		XStxClientRecycle(mClientHandle);
	}

	XStxResult result = XSTX_RESULT_OK;
	// setup xstx client and start making the connection
	if ((result = XStxClientCreate(mClientLibraryHandle, &mClientHandle))
		!= XSTX_RESULT_OK)
	{
		LOGE("Failed to create client.");
		const char *name; const char *desc;
		XStxResultGetInfo(result, &name, &desc);
		LOGE("XStxClientCreate failed with: %s", name);
		return result;
	}

	mStxListener.mReadyFcn = &::clientReady;
	mStxListener.mReadyCtx = this;
	mStxListener.mReconnectingFcn = &::clientReconnecting;
	mStxListener.mReconnectingCtx = this;
	mStxListener.mReconnectedFcn = &::clientReconnected;
	mStxListener.mReconnectedCtx = this;
	mStxListener.mStoppedFcn = &::clientStopped;
	mStxListener.mStoppedCtx = this;
	mStxListener.mMessageReceivedFcn = &::messageReceived;
	mStxListener.mMessageReceivedCtx = this;
	mStxListener.mStreamQualityMetricsReceivedFcn = &::clientQoS;
	mStxListener.mStreamQualityMetricsReceivedCtx = this;
	mStxListener.mSetConfigurationFcn = &::receivedClientConfiguration;
	mStxListener.mSetConfigurationCtx = this;
	mStxListener.mSize = sizeof(mStxListener);

	if ((result = XStxClientSetListener2(mClientHandle, &mStxListener))
		!= XSTX_RESULT_OK)
	{
		LOGE("Failed to set listener");
		const char *name; const char *desc;
		XStxResultGetInfo(result, &name, &desc);
		LOGE("XStxClientSetListener failed with: %s", name);
		platformErrorMessage(true, desc);
		return result;
	}

	// initialize video module
	if (!mVideoModule.initialize(mClientHandle, *mVideoRenderer))
	{
		LOGE("Failed to create video decoder/renderer");
		return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
	}

	// initialize audio module
	//if (0) 
	if (!mAudioModule.initialize(mClientHandle))
	{
		LOGE("Failed to create audio decoder/renderer");
		return XSTX_RESULT_NOT_INITIALIZED_PROPERLY;
	}

	if ((result = XStxClientSetEntitlementUrl(mClientHandle, address.c_str()))
		!= XSTX_RESULT_OK)
	{
		const char *name; const char *desc;
		XStxResultGetInfo(result, &name, &desc);
		LOGE("XStxClientSetEntitlementUrl failed with: %s", name);
		return result;
	}

	// non-blocking!
	/* result = XStxClientStart(mClientHandle);
	if ( (result == XSTX_RESULT_OK)
		|| ( result = XSTX_RESULT_AUDIO_FRAME_ALLOCATOR_NULL ) ) */
	//{
	//} else 
	if ((result = XStxClientStart(mClientHandle)) != XSTX_RESULT_OK)
	{
        LOGE("Failed to start client.");
        const char *name; const char *desc;
        XStxResultGetInfo(result, &name, &desc);
        LOGE("XStxClientStart failed with: %s", name);
        return result;
    }

    // success !
    LOGI("Started STX Thread");
    return result;
}

void AppStreamWrapper::step()
{
    if (mPaused)
    {
        return;
    }
    uint64_t startTime = mud::TimeVal::mono().toMilliSeconds();
    if (mVideoRenderer->isInitialized())
    {
        mFrameCount += mVideoRenderer->draw();

        uint64_t timePassed = (mud::TimeVal::mono() - mLastFrameRateStart).toMilliSeconds();
        if (timePassed > 1000)
        {
            // reset frame rate calculation
            mLastFrameRateStart = mud::TimeVal::mono();
            mLastTotalCount = mFrameCount;
            mFrameCount = 0;
            mTimePassedInSeconds = timePassed / 1000.0f;
#if APPSTREAM_ENABLE_METRICS
            mud::ScopeLock scope(mMetricsLock);
            mLastMetricsCount = mMetricsCount ;
            memcpy(&mLastMetrics,&mMetricsAccum,sizeof(mLastMetrics)) ;
            mMetricsCount = 0;

            memset(&mMetricsAccum,0,sizeof(mMetricsAccum));

            char metricsText[APPSTREAM_METRICS_BUFFER_SIZE];
            if (mLastMetricsCount)
            {
                mud::AStdio::snprintf(metricsText,APPSTREAM_METRICS_BUFFER_SIZE,
                                      "{ \"FPS\":%2.f, \"MeanPsnr\":%d%s, \"MeanLatency\":%d%s, \"MeanDistorted\":%d%s,  }",
                                      mLastTotalCount / mTimePassedInSeconds, // FPS
                                      mLastMetrics.mMeanPsnrDb/mLastMetricsCount, mLastMetrics.mMeanPsnrThresholdUnmet?" [threshold unmet]":"",
                                      mLastMetrics.mMeanLatencyMilliseconds/mLastMetricsCount, mLastMetrics.mMeanLatencyThresholdUnmet?" [threshold unmet]":"",
                                      mLastMetrics.mMeanDistortedFramesPercentage/mLastMetricsCount, mLastMetrics.mMeanDistortedFramesThresholdUnmet?" [threshold unmet]":""
                                      );
            }
            else
            {
                mud::AStdio::snprintf(metricsText,APPSTREAM_METRICS_BUFFER_SIZE,
                                      "{ \"FPS\":%2.f, \"noqos\":1 }",
                                      mLastTotalCount / mTimePassedInSeconds ) ;// FPS
            }
            LOGV("[metrics]=%s", metricsText);
#else
            // render frame-per-second
            char fpsText[6];
            mud::AStdio::snprintf(fpsText, 6, "%.2f", mLastTotalCount / mTimePassedInSeconds);
            LOGV("FPS: %s", fpsText);
#endif
        }

        mVideoRenderer->post();
    }
    else
    {
        mVideoRenderer->clearScreen();
    }
    uint64_t finishTime = mud::TimeVal::mono().toMilliSeconds();
    uint32_t totalTime = (uint32_t)(finishTime - startTime);

    // We're trying for 30FPS, so anything greater than 33ms is a problem
    if (totalTime > 32)
    {
        LOGV("step time: %dms", totalTime);
    }
}

XStxResult AppStreamWrapper::clientReady()
{
    LOGI("clientReady");

    // Reset the client streaming state
    mStreamingState = -1;

    platformOnConnectSuccess();
    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::clientReconnecting(
                            uint32_t timeoutMs,
                            XStxDisconnectReason reason)
{
    const char* name = XStxDisconnectReasonGetName(reason);
    const char* description = XStxDisconnectReasonGetDescription(reason);
    char message[256];
    mud::AStdio::snprintf(message, sizeof(message), "%s [%s]", description, name);

    LOGW("clientReconnecting: %u (%s)", timeoutMs, message);

    mReconnecting = true;

    pausePlayback(mPaused || mReconnecting);

    platformOnReconnecting(timeoutMs, message);

    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::clientReconnected()
{
    LOGW("clientReconnected");

    mReconnecting = false;

    pausePlayback(mPaused || mReconnecting);

    platformOnReconnected();

    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::clientStopped(XStxStopReason stopReason)
{
    // First thing: Set up the stop reason in streaming state.
    mStreamingState = stopReason;
    
    const char *name = XStxStopReasonGetName(stopReason);
    const char *description = XStxStopReasonGetDescription(stopReason);

    bool fatal =
        stopReason != XSTX_STOP_REASON_SESSION_REQUEST_FAILED &&
        stopReason != XSTX_STOP_REASON_SESSION_REQUEST_INVALID_ENTITLEMENT_URL &&
        stopReason != XSTX_STOP_REASON_SESSION_REQUEST_NO_AVAILABLE_INSTANCE &&
        stopReason != XSTX_STOP_REASON_SESSION_REQUEST_TIMED_OUT;

    LOGW("clientStopped : %s [%s] (%s)", description, name, fatal ? "fatal" : "non-fatal");

    char message[256];
    mud::AStdio::snprintf(message, sizeof(message), "%s [%s]", description, name);
    platformErrorMessage(fatal, message);

    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::messageReceived(const unsigned char* message, uint32_t length)
{
    // message arrived !
    std::string msg((const char *)message, length);
    LOGI("Received message: %s", msg.c_str());
    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::clientQoS(const XStxStreamQualityMetrics* metrics)
{
#if APPSTREAM_ENABLE_METRICS
    mud::ScopeLock scope(mMetricsLock);

    mMetricsAccum.mMeanDistortedFramesPercentage += metrics->mMeanDistortedFramesPercentage ;
    mMetricsAccum.mMeanDistortedFramesThresholdUnmet = mMetricsAccum.mMeanDistortedFramesThresholdUnmet || metrics->mMeanDistortedFramesThresholdUnmet ;

    mMetricsAccum.mMeanLatencyMilliseconds += metrics->mMeanLatencyMilliseconds ;
    mMetricsAccum.mMeanLatencyThresholdUnmet = mMetricsAccum.mMeanLatencyThresholdUnmet || metrics->mMeanLatencyThresholdUnmet ;

    mMetricsAccum.mMeanPsnrDb += metrics->mMeanPsnrDb ;
    mMetricsAccum.mMeanPsnrThresholdUnmet = mMetricsAccum.mMeanPsnrThresholdUnmet || metrics->mMeanPsnrThresholdUnmet ;

    mMetricsCount ++ ;
#endif

    return XSTX_RESULT_OK;
}

XStxResult AppStreamWrapper::receivedClientConfiguration(const XStxClientConfiguration* config)
{
    // client configuration settings received
    // relay to video module

    LOGI("Received client configuration");
    mVideoModule.receivedClientConfiguration(config);

    return XSTX_RESULT_OK;
}

void AppStreamWrapper::initGraphics(int w, int h, bool forceRescale)
{
    mVideoRenderer->init();
    mVideoRenderer->setDisplayDimensions(w, h, forceRescale);
}

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//
//          XStx example client C callbacks
//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

XStxResult clientReady(void *context,
                uint32_t size,
                const uint8_t* buffer)
{
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->clientReady();
}

XStxResult clientReconnecting(
                    void *context,
                    uint32_t timeoutMs,
                    XStxDisconnectReason reason)
{
    LOGV("clientReconnecting");
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->clientReconnecting(timeoutMs, reason);
}

XStxResult clientReconnected(void *context)
{
    LOGV("clientReconnected");
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->clientReconnected();
}

XStxResult clientStopped(void *context,  XStxStopReason stopReason)
{
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->clientStopped(stopReason);
}

XStxResult messageReceived(void *context, const unsigned char *message, uint32_t length)
{
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->messageReceived(message, length);
}

XStxResult clientQoS(void *context, const XStxStreamQualityMetrics* metrics)
{
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->clientQoS(metrics);
}

XStxResult receivedClientConfiguration(void *context, const XStxClientConfiguration* config)
{
    AppStreamWrapper *module = (AppStreamWrapper *)context;
    return module->receivedClientConfiguration(config);
}

void AppStreamWrapper::keyPress(int key, bool down)
{
    XStxInputEvent xstxevent = { 0 };
    xstxevent.mTimestampUs = mud::TimeVal::mono().toMilliSeconds();
    xstxevent.mType = XSTX_INPUT_EVENT_TYPE_KEYBOARD;
    xstxevent.mInfo.mKeyboard.mVirtualKey = key;
    xstxevent.mInfo.mKeyboard.mIsKeyDown = down;
    xstxevent.mSize = sizeof(XStxInputEvent);

    sendInput(xstxevent);
}

void AppStreamWrapper::mouseEvent(int x, int y, uint32_t flags)
{
    // no video renderer? Return!
    if (!mVideoRenderer)
    {
        return;
    }

    int xOff, yOff;
    float scale;
    mVideoRenderer->getScaleAndOffset(scale, xOff, yOff);

    // no scale yet; ignore!
    if (scale == 0) return;

    XStxInputEvent xstxevent = { 0 };
    xstxevent.mTimestampUs = mud::TimeVal::mono().toMilliSeconds();
    xstxevent.mType = XSTX_INPUT_EVENT_TYPE_MOUSE;
    xstxevent.mInfo.mMouse.mButtonFlags = flags;
    xstxevent.mInfo.mMouse.mFlags = 1; //absolute

    static int lastX = 0, lastY = 0;

    if (flags & CET_MOUSE_WHEEL)
    {
        LOGV("Mouse wheel data: %d", x);
        // mouse wheel data goes in mButtonData.
        xstxevent.mInfo.mMouse.mButtonData = x;
        xstxevent.mInfo.mMouse.mLastX = lastX;
        xstxevent.mInfo.mMouse.mLastY = lastY;
    }
    else
    {
        lastX = xstxevent.mInfo.mMouse.mLastX = (int)((x + xOff) * scale);
        lastY = xstxevent.mInfo.mMouse.mLastY = (int)((y + yOff) * scale);
//        LOGV("Mouse data: x:%d,y:%d  (from %d,%d)", lastX, lastY,x,y);
    }

    sendInput(xstxevent);
}

// send a message using XStxClientSendMessage api call
XStxResult AppStreamWrapper::sendMessage( const unsigned char * message, uint32_t length)
{
    return XStxClientSendMessage(mClientHandle,message,length);
}

void AppStreamWrapper::setKeyboardOffset(int offset)
{
    if (mVideoRenderer)
    {
        mVideoRenderer->setKeyboardOffset(offset);
    }
}

void AppStreamWrapper::pause(bool pause)
{
    LOGV("pause %d", pause);

    mPaused = pause;
    pausePlayback(mPaused || mReconnecting);
}

void AppStreamWrapper::stop()
{
    LOGV("stop");
    mVideoModule.stop();

    if ( 0 ) //if (mAudioModule.getRenderer())
    {
        LOGV("mAudioModule.getRenderer() non-null; calling stop");
        mAudioModule.getRenderer()->stop();
    }

    if (mClientHandle)
    {
        LOGV("calling XStxClientStop");
        XStxClientStop(mClientHandle);
    }
}


// Send input to server using predefined input event structure.
void AppStreamWrapper::sendInput(XStxInputEvent event)
{
    XStxClientSendInput(mClientHandle, &event);
}


void AppStreamWrapper::sendRawInput(const uint8_t *inputString, int length)
{
    XStxClientSendRawInput(mClientHandle, inputString, length);
}

XStxResult AppStreamWrapper::recycle()
{
    if (mClientHandle == NULL && mClientLibraryHandle == NULL)
    {
        return XSTX_RESULT_INVALID_STATE;
    }

    // Tell the video renderer to stop so that XStx doesn't
    // deadlock waiting for it to exit.
    if (mVideoRenderer)
    {
        mVideoRenderer->stop();
    }

    // Recycle both XStx client handles, regardless of errors. We need to clean up
    // deterministically.
    XStxResult result = XSTX_RESULT_OK;
    XStxResult result2 = XSTX_RESULT_OK;

    if (mClientHandle!=NULL)
    {
        result = XStxClientRecycle(mClientHandle);
    }
    if (mClientLibraryHandle!=NULL)
    {
        result2 = XStxClientLibraryRecycle(mClientLibraryHandle);
    }

    mClientHandle = NULL;
    mClientLibraryHandle = NULL;

    if (result != XSTX_RESULT_OK)
    {
        return result;
    }

    return result2;
}

/** Pause audio/video decoding/rendering. */
void AppStreamWrapper::pausePlayback(bool pause)
{
    LOGV("pausePlayback %d", pause);

    mVideoModule.pause(pause);

    if (mAudioModule.getRenderer())
    {
        LOGV("AudioRenderer::pause");

        mAudioModule.getRenderer()->pause(pause);
    }
}

void AppStreamWrapper::joystickState(
    uint16_t     buttons,
    uint8_t      leftTrigger,
    uint8_t      rightTrigger,
    int16_t      thumbLX,
    int16_t      thumbLY,
    int16_t      thumbRX,
    int16_t      thumbRY)
{
    if (!mClientHandle)
    {
        return;
    }

    XStxInputEvent ie = {0};
    ie.mSize = sizeof(ie);
    ie.mTimestampUs = mud::TimeVal::mono().toMilliSeconds();
    ie.mType = XSTX_INPUT_EVENT_TYPE_GAMEPAD;

    XStxGamepadInfo & gamepad = ie.mInfo.mGamepad;

    gamepad.mButtons = buttons;
    gamepad.mLeftTrigger = leftTrigger;
    gamepad.mRightTrigger = rightTrigger;
    gamepad.mThumbLX = thumbLX;
    gamepad.mThumbLY = thumbLY;
    gamepad.mThumbRX = thumbRX;
    gamepad.mThumbRY = thumbRY;

    if (memcmp(&gamepad,&mLastJoystickState,sizeof(XStxGamepadInfo))==0)
    {
        // no change; we're done
        return;
    }

    memcpy(&mLastJoystickState,&gamepad,sizeof(gamepad));
    sendInput(ie);
}

