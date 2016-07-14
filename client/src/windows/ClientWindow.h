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

#include <windows.h>
#include "MUD/threading/Thread.h"
#include "MUD/threading/SimpleLock.h"
#include "MUD/threading/ScopeLock.h"

#include "DES/AppStreamClientHelpers.h"

#include "AppStreamWrapper.h"

static const char * WINDOW_TITLE = "Amazon AppStream Simple Client";

/* This class set up a client Window that:
 * - has an AppStreamWrapper object to start/end the connection and streaming
 * - has a thread to call renderLoop which uses AppStreamWrapper to render(AppStreamWrapper in turn, uses a directXRenderer)
 * - start an eventLoop which
*/
class ClientWindow
{
private:
    DEFINE_METHOD_THREAD(DrawThread, ClientWindow, renderLoop);

    //handle of the window created.
    HWND mWindow;
    AppStreamWrapper * mAppStreamWrapper;

    uint32_t mWidth, mHeight;
    bool mIsFullScreen;
    bool mResizeNeeded;

    bool mShouldExit;
    mud::SimpleLock mExitLock;
    mud::SimpleLock mRunningLock;
    // for rescaling our mouse when we send absolute movements
    int mWindowBorderAdjustWidth;
    int mWindowBorderAdjustHeight;
    DrawThread dt;

    /** entitlement URL **/
    std::string mEntitlementUrl;

    helpers::EntitlementInfo mEntitlementInfo;

    // prompt for entitlement
    bool mShouldPromptForEntitlement;
    std::string mEntitlementPromptErrorText;
    mud::SimpleLock mShouldPromptForEntitlementLock;

    //create a client window
    bool createWindowHandle(HWND & window);
    HWND createFullScreenWindow(HINSTANCE hInstance);
    HWND createWindowedWindow(HINSTANCE hInstance);
    /** event handler loop */
    void eventLoop();

    bool shouldShowEntitlementPrompt()
    {
        mud::ScopeLock sl(mShouldPromptForEntitlementLock);
        return mShouldPromptForEntitlement;
    }

    void getEntitlementPromptErrorText(std::string& errText)
    {
        mud::ScopeLock sl(mShouldPromptForEntitlementLock);
        errText = mEntitlementPromptErrorText;
    }

    //flag to tell eventLoop() to stop when user click the 'x' button
    //the return value of the eventLoop() will tell the owning RenderWindow
    //object to stop.
    mud::SimpleLock mStopDrawingLock;
    bool mStopDrawing;

    //to check if directXRenderer has finished initializing
    mud::SimpleLock mGraphicsInitializedLock;
    bool mGraphicsInitialized;
    bool isGraphicsInitialized();
public:

    /** Constructor */
    ClientWindow();

    /** Destructor */
    ~ClientWindow();

    void init();
    /**
     * Set width and height
     * @param[in] w width
     * @param[in] h height
     */
    void setDimensions(uint32_t w, uint32_t h);

    /**
     * Full screen mode
     * @param[in] isFullscreen
     */
    void setFullscreen(bool isFullscreen);

    /**
     * Set the AppStreamWrapper object
     */
    void setAppStreamWrapper(AppStreamWrapper * AppStreamWrapper);

    int getBorderWidth()
    {
        return mWindowBorderAdjustWidth;
    }

    int getBorderHeight()
    {
        return mWindowBorderAdjustHeight;
    }

    /** Initialize and then Show the clientWindow, this will kick of connection to the server and
      * utilize DirectX render to render the frames
      * Blocks for the duration of the window being open.
      */
    bool beginDisplay();

    /** stop event loop*/
    void stop();
    bool shouldExit() ;

    /** stop render loop*/
    void stopDrawing();
    bool shouldStopDrawing() ;

    /**
     * Video render loop runs on a separate thread
     * It delegates rendering to AppStreamWrapper using its step() function
     */
    void renderLoop();

    HWND getWindowHandle()
    {
        return mWindow;
    }

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

    /**
     * Enable flag to let render window know to prompt for
     * entitlement on next run of event loop.
     */
    void setShouldShowEntitlementPrompt(const char* errText)
    {
        mud::ScopeLock sl(mShouldPromptForEntitlementLock);
        mEntitlementPromptErrorText = errText;
        mShouldPromptForEntitlement = true;
        //post a null message to break out of blocking GetMessage( so that new entitlement window
        // can be displayed)
        ::PostMessage(mWindow, WM_NULL, 0, 0);
    }

    void completedEntitlementPrompt()
    {
        mud::ScopeLock sl(mShouldPromptForEntitlementLock);
        mShouldPromptForEntitlement = false;
    }
};


#endif
