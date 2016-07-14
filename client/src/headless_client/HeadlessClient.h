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


#ifndef APPSTREAM_CLIENT_WINDOW_H
#define APPSTREAM_CLIENT_WINDOW_H

#include "MUD/threading/Thread.h"
#include "MUD/threading/SimpleLock.h"
#include "MUD/threading/ScopeLock.h"

#include "AppStreamClientHelpers.h"

#include "AppStreamWrapper.h"

static const char * WINDOW_TITLE = "Amazon AppStream Simple Client";

/* This class set up a client Window that:
 * - has an AppStreamWrapper object to start/end the connection and streaming
 * - has a thread to call renderLoop which uses AppStreamWrapper to render(AppStreamWrapper in turn, uses a directXRenderer)
 * - start an eventLoop which
*/
class HeadlessClient
{
private:
    DEFINE_METHOD_THREAD(DrawThread, HeadlessClient, renderLoop);

    AppStreamWrapper * mAppStreamWrapper;

    DrawThread dt;

    /** entitlement URL **/
    std::string mEntitlementUrl;

    helpers::EntitlementInfo mEntitlementInfo;

    /**
     * this loop listens for input from stdin.
     * it also check AppStreamWrapper and quit if AppStreamWrapper
     * informs that streaming session has stopped, or failed to
     * start.
     * returns error code when streaming has stopped
     */
     /*this is a little difference from normal Windows client because for normal
     / window client:
      - the user will close the client, the client never closes itself
      - this mean a thing outside of this class will terminate the event loop and render loop
      - for that reason we have the public stop() and shouldExit()(used inside eventloop)
       function in the normal client but not here.
      - the render loop may need to keep running event after eventloop stops, to refresh
       the window when user move, resize...
       */
     //TODO: may need to reconsider how and when to terminate render loop in normal client.
    int32_t eventLoop();

    //flag to tell eventLoop() to stop when user click the 'x' button
    //the return value of the eventLoop() will tell the owning RenderWindow
    //object to stop.
    mud::SimpleLock mStopDrawingLock;
    bool mStopDrawing;
    /** stop render loop */
    void stopDrawing();
    bool shouldStopDrawing() ;

public:
    /** Constructor */
    HeadlessClient();
    /** Destructor */
    ~HeadlessClient();

    /**
     * - tells the AppStreamWrapper to start a streaming session
     * - starts the video render thread and event loop
     * returns the error code when streaming session ends
     */
    int32_t beginDisplay();

    void setAppStreamWrapper(AppStreamWrapper * AppStreamWrapper);

    /**
     * Video render loop runs on a separate thread
     * It delegates rendering to AppStreamWrapper using its step() function
     */
    void renderLoop();

    /**
     * Set entitlement url
     * @param[in] entitlementUrl entitlement url
     */
    void setEntitlementUrl(const char* entitlementUrl)
    {
        mEntitlementUrl = entitlementUrl;
    }

    /**
     * Set pre-populated entitlement information
     */
    void setEntitlementInfo(const helpers::EntitlementInfo& entitlementInfo)
    {
        mEntitlementInfo.serverUrl = entitlementInfo.serverUrl;
        mEntitlementInfo.identityToken = entitlementInfo.identityToken;
        mEntitlementInfo.applicationId = entitlementInfo.applicationId;
    }
};


#endif
