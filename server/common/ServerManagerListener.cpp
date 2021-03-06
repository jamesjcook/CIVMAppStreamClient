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

#include <map>

#include "XStx/common/XStxUtil.h"

#include "ServerManagerListener.h"
#include "HostedApplication.h"

/**                     Media sources and timestamps
 * Source:
 * The audio source provided by the hosted application is optional. When an audio 
 * source is not specified via XStxServerSetAudioSource, the built-in audio capture
 * will be used. The video source provided by the hosted application, is required.
 * Timestamp:
 * If the built-in audio capture is used, the SDK timestamps the audio frames using
 * a built-in timestamp manager. The video timestamps can also be set automatically 
 * in this case (see the comment int XStxServerAPI for the setVideoSource function). 
 * The application could still set timestamps in the case when built-in audio 
 * capture is used, however. In this case it can use the API: XStxServerGetTimestampUs
 * to query the SDK for the current media time. The application would choose 
 * to provide video timestamps when audio timestamps are generated automatically.
 * For example, if there is a known time offset between audio and video coming 
 * from the application. 
 */

/**
 * This preprocessor could be commented out to switch to built-in audio capture for
 * this example.
*/
//#define APPLICATION_CAPTURES_AUDIO 1

#define XSTX_CALLBACK_NOT_NULL_OR_ERROR(inst, callback) \
    if (inst->m##callback##Fcn == NULL)                 \
    {                                                   \
        return XSTX_RESULT_INVALID_ARGUMENTS;           \
    }

/**
 * Example impementation of XStxIServerManagerListener. 
 *
 * The static 'Imp' derivation is used to hide impementation
 * details from consumers of ServerManagerListener.
 */
struct ServerManagerListenerImp
    :
    public ServerManagerListener,
    private XStxIServerManagerListener
{
public:

    ServerManagerListenerImp(XStxServerLibraryHandle serverLibraryHandle)
        :
        mServerLibraryHandle(serverLibraryHandle)
    {
        /** Initilaize the XStx interface */

        XSTX_INIT_INTERFACE_SIZE(XStxIServerManagerListener)
        XSTX_INIT_CALLBACK(XStxIServerManagerListener, ServerInitialize)
        XSTX_INIT_CALLBACK(XStxIServerManagerListener, ServerSaveState)
        XSTX_INIT_CALLBACK(XStxIServerManagerListener, ServerTerminate)
    }

    XStxIServerManagerListener* getServerManagerListener()
    {
        return this;
    }

private:

    /**
     * Create static methods satisfying the XStx interfaces and declare
     * the corresponding non-static methods.
     */
    
    XSTX_DECLARE_CALLBACK_3(
        ServerManagerListenerImp,
        XStxIServerManagerListenerServerInitialize,
        XStxServerHandle,
        uint32_t,
        const char*)
    XSTX_DECLARE_CALLBACK_3(
        ServerManagerListenerImp,
        XStxIServerManagerListenerServerSaveState,
        XStxServerHandle,
        uint32_t,
        XStxStopReason)
    XSTX_DECLARE_CALLBACK_3(
        ServerManagerListenerImp,
        XStxIServerManagerListenerServerTerminate,
        XStxServerHandle,
        uint32_t,
        XStxStopReason)
    XSTX_DECLARE_CALLBACK_0(
        ServerManagerListenerImp,
        XStxIServerManagerListenerRecycle)

    struct ServerInfo
    {
        HostedApplication* mApp;
        XStxServerHandle mServer;
    };

    typedef std::map< XStxServerHandle,  ServerInfo* > ServerToInfoMap;

    /** Instance data */

    XStxServerLibraryHandle mServerLibraryHandle;

    ServerToInfoMap mServerToInfoMap;
};

XStxResult ServerManagerListenerImp::XStxIServerManagerListenerServerInitialize(
    XStxServerHandle server,
    uint32_t timeout,
    const char* applicationContext)
{
    /** Check if the server is running */

    if (mServerToInfoMap.find(server) != mServerToInfoMap.end())
    {
        return XSTX_RESULT_ALREADY_CREATED;
    }

    XStxResult result = XSTX_RESULT_OK;

    /** Create an object to hold on to server specific info */

    ServerInfo* info = new ServerInfo();
    XStxIServerListener2* listener = NULL;
    info->mApp = NULL;
    info->mServer = server;

    /** Intiantiate the hosted application */
    
    result = HostedApplication::createHostedApplication(
        server,
        applicationContext,
        info->mApp);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /** Point the app to the server and start the app */

    result = info->mApp->setServer(info->mServer);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    result = info->mApp->start();

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /** Point the server to the app and start the server */

    listener = info->mApp->getServerListener();
    if ( NULL != listener )
    {
        XSTX_CALLBACK_NOT_NULL_OR_ERROR(listener, Ready);
        XSTX_CALLBACK_NOT_NULL_OR_ERROR(listener, Reconnecting);
        XSTX_CALLBACK_NOT_NULL_OR_ERROR(listener, Reconnected);
        XSTX_CALLBACK_NOT_NULL_OR_ERROR(listener, Stopped);
        XSTX_CALLBACK_NOT_NULL_OR_ERROR(listener, MessageReceived);
    }
    result = XStxServerSetListener2(
        info->mServer,
        listener);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }
    
    result = XStxServerSetInputSink(
        info->mServer,
        info->mApp->getInputSink());

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }
#ifdef APPLICATION_CAPTURES_AUDIO
    //The example audio source does provides timestamps
    result = XStxServerSetAudioSource(
        info->mServer,
        info->mApp->getAudioSource(), true);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }
    //need to manually change the isProvidingTimestamp flag here to false
    //if it doesn't provide timestamp
    result = XStxServerSetVideoSource(
        info->mServer,
        info->mApp->getVideoSource(), true);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }
#else
    //need to manually change the isProvidingTimestamp flag here to false
    //if it doesn't provide timestamp
    result = XStxServerSetVideoSource(
        info->mServer,
        info->mApp->getVideoSource(), false);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }
#endif

    mServerToInfoMap[server] = info;
    if(std::getenv("XSTX_EXAMPLE_SERVER_SET_FPS") != NULL)
    {
        int targetFps = atoi(std::getenv("XSTX_EXAMPLE_SERVER_SET_FPS"));
        XStxServerSetFixedFrameRate(info->mServer, double(targetFps));
    }
    return XSTX_RESULT_OK;

exit:

    if (info != NULL)
    {
        delete info->mApp;
        XStxServerRecycle(info->mServer);
        delete info;
    }

    return result;
}

XStxResult ServerManagerListenerImp::XStxIServerManagerListenerServerSaveState(
    XStxServerHandle session,
    uint32_t timeout,
    XStxStopReason reason)
{
    return XSTX_RESULT_OK;
}

XStxResult ServerManagerListenerImp::XStxIServerManagerListenerServerTerminate(
    XStxServerHandle session,
    uint32_t timeout,
    XStxStopReason reason)
{
    ServerToInfoMap::iterator it = mServerToInfoMap.find(session);

    if (mServerToInfoMap.end() == it)
    {
        return XSTX_RESULT_INVALID_HANDLE;
    }

    ServerInfo* info = it->second;
    mServerToInfoMap.erase(it);

    if (info != NULL)
    {
        XStxServerRecycle(info->mServer);
        delete info->mApp;
        delete info;
    }

    return XSTX_RESULT_OK;
}

XStxResult ServerManagerListenerImp::XStxIServerManagerListenerRecycle()
{
    delete this;
    return XSTX_RESULT_OK;
}

XStxResult ServerManagerListener::createServerManagerListener(
        XStxServerLibraryHandle serverLibraryHandle,
        ServerManagerListener*& serverManagerListener)
{
    serverManagerListener = new ServerManagerListenerImp(serverLibraryHandle);

    if (NULL == serverManagerListener)
    {
        return XSTX_RESULT_OUT_OF_MEMORY;
    }

    return XSTX_RESULT_OK;
}
