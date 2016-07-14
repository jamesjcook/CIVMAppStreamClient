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

#include "HeadlessClient.h"
#include <iostream>
#define IDR_MAINFRAME                   128

/** Constructor */
HeadlessClient::HeadlessClient()
                    : mAppStreamWrapper(NULL),
                      dt("HeadlessClient", *this),
                      mStopDrawing(false)
{

}

/** Destructor */
HeadlessClient::~HeadlessClient()
{

}


void HeadlessClient::setAppStreamWrapper(AppStreamWrapper * appstreamwrapper)
{
    mAppStreamWrapper = appstreamwrapper;
}


int32_t HeadlessClient::beginDisplay()
{
    if (mEntitlementUrl.empty())
    {

       // No entitlement URL retrieved
        printf("\nError: No entitlement url found.");
        return XSTX_STOP_REASON_SESSION_REQUEST_INVALID_ENTITLEMENT_URL;
    }

    dt.start();

    mAppStreamWrapper->connect(mEntitlementUrl);

    int32_t errorCode = eventLoop();

    //when event loop finishes, stop render thread as well
    stopDrawing();
    return errorCode;
}

void HeadlessClient::renderLoop()
{
    mAppStreamWrapper->initGraphics(1280,720);
    while (!shouldStopDrawing())
    {
        mAppStreamWrapper->step();
    }

}

/** stop render loop */
void HeadlessClient::stopDrawing()
{
    {
        mud::ScopeLock sl(mStopDrawingLock);
        mStopDrawing = true;
    }
    dt.join();
}
bool HeadlessClient::shouldStopDrawing()
{
    mud::ScopeLock sl(mStopDrawingLock);
    return mStopDrawing;
}


/** event handler loop */
int32_t HeadlessClient::eventLoop()
{
    int32_t errorCode;
    while (1)
    {
        //check for input from stdin
       /* char x;
        std::cin >> x;
        if (x =='q')
            {
                printf("\nHeadless client was terminated manually.");

                return XSTX_STOP_REASON_REQUESTED;
            }*/

        //query AppStreamWrapper and check if streaming session has stoppped
        //(or failed to start)
        if(mAppStreamWrapper->isStreamingStopped(errorCode))
        {
                return errorCode;
        }

    }
}


