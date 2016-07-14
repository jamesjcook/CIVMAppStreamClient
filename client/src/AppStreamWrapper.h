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


#ifndef _included_AppStreamWrapper_h
#define _included_AppStreamWrapper_h

#include <stdint.h>
#include <XStx/client/XStxClientAPI.h>
#include <XStx/common/XStxAPI.h>
#include <MUD/base/TimeVal.h>
#include <string>

#include "VideoRenderer.h"
#include "VideoModule.h"
#include "AudioModule.h"

#include "platformBindings.h"

/// Enable metrics calculations and output to the log.
#define APPSTREAM_ENABLE_METRICS 1
#define APPSTREAM_METRICS_BUFFER_SIZE 400

/**
 * A class that registers and handles up the main callbacks from
 * AppStream. Video and audio callbacks are registered in the VideoModule
 * and AudioModule classes, respectively.
 *
 * This is the first class to understand when you're looking at any of the
 * example clients; most of the logic necessary for the top-level control
 * of XStx is coded here. In addition it would be valuable to understand
 * VideoModule and AudioModule, the classes that handle decoding and
 * rendering of video and audio, respectively.
 *
 * Formerly called XStxModule, but this caused some confusion, and so it
 * was renamed.
 */
class AppStreamWrapper
{
public:
    /**
     * Constructor.
     */
    AppStreamWrapper();

    /**
     * Destructor.
     */
    virtual ~AppStreamWrapper();

    /**
     * Initialize the AppStreamWrapper.
     *
     * @return XSTX_RESULT_OK on success.
     */
    XStxResult init();

    /**
     * Stop the client gracefully. This is called when the application is
     * completely hidden. The application may still be restored after stop()
     * is called.
     */
    void stop();

    /**
     * Connect to the AppStream server.
     *
     * The address can be in one of these forms:
     *
     * * ssm://123.123.123.123:port/?sessionId=%s -- connect to a server using
     *   AppStream Stand-Alone Mode.
     *
     * * http[s]://domain.name:port/api/entitlements/11111111-...(your appid)
     *   to connect to an entitlement server.
     *
     * @param[in] address Address to connect with or full entitlement URL.
     *
     * @return XSTX_RESULT_OK on success or the last error.
     */
    XStxResult connect(std::string address);

    /**
     * Recycle any contained XStxClientHandle or XStxClientLibraryHandle.
     *
     * @return XSTX_RESULT_OK on success or the last error.
     */
    XStxResult recycle();

    /**
     * Step the video render. Called by the platform for each frame that is to
     * be rendered.
     */
    void step();

    /**
     * Initialize the graphics subsystem, with the given display dimensions.
     *
     * @param[in] x      Width of the display in pixels.
     * @param[in] y      Height of the display in pixels.
     * @param[in] forceRescale Force the screen to be rescaled to the new
     *       size. On Android, bringing up the keyboard resizes the screen,
     *       but we don't want it to rescale.
     */
    void initGraphics(int x, int y, bool forceRescale=true);

    /**
     * Send a key to the server.
     *
     * When communicating to a Windows server, \c key should be a virtual
     * keycode. In particular, alphabetic keys are represented by their
     * respective capital letters, and numeric keys are represented by digits.
     * Some keys have special codes; see the following web site for a list:
     *
     * http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731%28v=vs.85%29.aspx
     *
     * If you need a "$" character, for instance, you will need
     * to send VK_SHIFT (down), '4' (down), '4' (up), VK_SHIFT
     * (up). For a '?' character, you need to send
     * VK_SHIFT (down), VK_OEM2 (down), VK_OEM2 (up), VK_SHIFT
     * (up).
     *
     * On Android, the keys are typically sent using this
     * pattern, but it can be important to be aware of the
     * pattern if you're trying to trigger a specific key
     * programmatically.
     *
     * @param[in] key    Keycode to send.
     *
     * @param[in] down   True to send a key-down message; false to send
     *               key-up.
     */

    void keyPress(int key, bool down);

    /**
     * Send a mouse event to the server.
     *
     * @param[in] x      X coordinate of the mouse, OR the relative mouse
     *                   wheel change.
     * @param[in] y      Y coordinate of the mouse (or 0 for mouse wheel).
     * @param[in] flags  Flags, set from \ref MouseFlags.
     *
     * @see  CET_MOUSE_1_DOWN
     * @see  CET_MOUSE_1_UP
     * @see  CET_MOUSE_2_DOWN
     * @see  CET_MOUSE_2_UP
     * @see  CET_MOUSE_3_DOWN
     * @see  CET_MOUSE_3_UP
     * @see  CET_MOUSE_WHEEL
     * @see  CET_TOUCH_FLAG
     *
     */
    void mouseEvent(int x, int y, uint32_t flags);

    /**
     * Notification of the joystick state. May get multiple identical updates;
     * this implementation will only send updates to the server when the state
     * has changed.
     *
     * @param buttons      A logical OR of all of the buttons currently active.
     * @param leftTrigger  Left trigger, expressed as 0-255.
     * @param rightTrigger Right trigger, expressed as 0-255.
     * @param thumbLX      Left Thumb, X Axis, -32768-32767.
     * @param thumbLY      Left Thumb, Y Axis, -32768-32767.
     * @param thumbRX      Right Thumb, X Axis, -32768-32767.
     * @param thumbRY      Right Thumb, Y Axis, -32768-32767.
     */
    virtual void joystickState(
        uint16_t     buttons,
        uint8_t      leftTrigger,
        uint8_t      rightTrigger,
        int16_t      thumbLX,
        int16_t      thumbLY,
        int16_t      thumbRX,
        int16_t      thumbRY);

    /**
     * Send an XStxInputEvent to the server
     * @param[in]       event  An XStxInputEvent structure to send
     *
     */
    void sendInput(XStxInputEvent event);

    /**
     * Send raw input to the server
     * @param[in]  inputData Arbitrary data
     * @param[in]  length Size of the data being sent
     *
     */
    void sendRawInput(const uint8_t *inputData, int length);

    /**
     * Enter pause state. Pause state indicates the application is still
     * visible, but a notification or dialog (or another application on
     * desktop operating systems) is partly blocking the application.
     * Depending on the implementation, you can choose to continue animating
     * and/or playing sound in that state.
     *
     * You could for instance decide to lower the volume in the pause state.
     * The default implementation on Android mutes the sound, for example.
     *
     * @param[in] pause  True to enter; false to exit.
     */
    void pause(bool pause);

    /**
     * A call that lets us know what offset we should use for the surface when
     * the keyboard is present.
     *
     * @param[in] offset Number of pixels to offset the view.
     */
    void setKeyboardOffset(int offset);

    /**
     * Client ready callback. Called by the XSTX code when the client
     * is ready.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult clientReady();

    /**
     * Client reconnecting callback. Called by the XSTX code when the client
     * has lost the connection to the server and is reconnecting.
     *
     * @param[in] timeoutMs The amount of time left (in milliseconds)
     * before the client will stop attempting to reconnect.
     *
     * @param[in] reason An value indictaing why the disconnect occured.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult clientReconnecting(
                                uint32_t timeoutMs,
                                XStxDisconnectReason reason);

    /**
     * Client reconnected callback. Called by the XSTX code when the client
     * has reconnected to the server.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult clientReconnected();

    /**
     * Client stopped callback. Called by the XSTX code when the client
     * has been stopped.
     *
     * @param[in] stopReason One of the XStxStopReason enumeration.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult clientStopped(XStxStopReason stopReason);

    /**
     * Send a message to the server.
     * @param[in] bytestream message to send
     * @param[in] length of the message being sent
     */
    virtual XStxResult sendMessage( const unsigned char * message, uint32_t length);

    /**
     * Message callback. Called by the XSTX code when the client
     * receives a message from the server.
     *
     * @param[in] message byte array representing the message.
     * @param[in] length The length of the message.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult messageReceived(const unsigned char *message, uint32_t length);

    /**
     * Client Quality of Service metrics callback.
     *
     * @param[in] info   Quality of service information.
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult clientQoS(const XStxStreamQualityMetrics* info);

    /**
     *
     * Client configuration settings from STX
     *
     * @param[in] config client configuration settings
     *
     * @return XSTX_RESULT_OK on success.
     */
    virtual XStxResult receivedClientConfiguration(const XStxClientConfiguration* config);

    /**
     * Returns the current VideoRenderer instance being used by this module to
     * render video frames.
     */

    VideoRenderer* getVideoRenderer() { return mVideoRenderer; }

    /**
     * return the current frames per second
     */
    float getFPS() { return (float)(mLastTotalCount / mTimePassedInSeconds); }

    /**
     * Return true if streaming session has stopped. This is used in the scenario
     * where a client wants to automatically close itself without requiring user actions.
     *
     * @param[out] errorCode the error code if streaming has stopped
     */
    bool isStreamingStopped(int32_t & errorCode)
    {
        if (mStreamingState==-1)
        {
            return false;
        }
        else
        {
            errorCode = mStreamingState;
            return true;
        }
    }

    /**
     *  Pause/resume audio/video decoding/rendering.
     *
     *  @param[in] pause the flag that indicates whether to pause or resume decoding/rendering.
     */
    void pausePlayback(bool pause);

protected :

    /**
     * The associated VideoModule that handles decode and rendering of
     * video.
     */
    VideoModule mVideoModule;

    /**
     * The AudioModule that controls our audio
     */
    AudioModule mAudioModule;

    /**
     * The associated video renderer (which we own instead of the VideoModule
     * so that we can create it early and query it for capabilities).
     */
    VideoRenderer *mVideoRenderer;

    /**
     * The set of XStx client callbacks.
     */
    XStxIClientListener2 mStxListener;

    /// The XStx client handle.
    XStxClientHandle mClientHandle;

    /// The XStx client library handle.
    XStxClientLibraryHandle mClientLibraryHandle;

    /**
     * Frame rate and metrics calculations. You can safely ignore these if
     * you're still trying to learn about XStx and AppStream.
     *
     * You can also delete them if you want to simplify the sample, but these
     * metrics can help you diagnose issues you may face in development, so
     * it's recommended that you leave them in place.
     *
     * @{
     */
    uint32_t mFrameCount;               ///< Number of frames in this measurement period.
    mud::TimeVal mLastFrameRateStart;   ///< Time this measurement period started.
    uint32_t mLastTotalCount;           ///< Total frames in previous measurement period.
    double mTimePassedInSeconds;        ///< Total time passed in previous measurement period.
#if APPSTREAM_ENABLE_METRICS                                        ///
    XStxStreamQualityMetrics mMetricsAccum;     ///< Accumulated metrics data.
    XStxStreamQualityMetrics mLastMetrics;      ///< Metrics data from previous measurement period.
    uint32_t mMetricsCount ;            ///< Number of accumulated metrics samples
    uint32_t mLastMetricsCount ;        ///< Number of metrics samples in previous measurement period.
    mud::SimpleLock mMetricsLock;       ///< A mutex that prevents the metrics from being printed in the midst
#endif                                  ///  of a calculation.
    /**@}*/

    /**
     * The streaming is currently paused.
     */
    bool mPaused;

    /**
     * The stream is currently reconnecting.
     */
    bool mReconnecting;

private:
    /**
     * The current state of the streaming.
     *
     * -1 if streaming is (potentially) active: ClientReady has been called,
     * and ClientStopped has not been called.
     *
     * Otherwise the "stopped reason" error code.
     */
    volatile int32_t mStreamingState;

    XStxGamepadInfo mLastJoystickState;
};

#endif // _included_AppStreamWrapper_h

