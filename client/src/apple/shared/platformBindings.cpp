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


#include "AppStreamClient.h"
#include "platformBindings.h"

#undef LOG_TAG
#define LOG_TAG "platformBindings"
#include "log.h"
#include "constants.h"


extern AppStreamWrapper *gAppStreamWrapper;

void step()
{
    assert(gAppStreamWrapper);
    gAppStreamWrapper->step();
}
void stop()
{
    assert(gAppStreamWrapper);
    gAppStreamWrapper->stop();
}

void connect(char *url)
{
    assert(gAppStreamWrapper);
    gAppStreamWrapper->connect(url);
}

void _setupGraphics(int w, int h)
{
    gAppStreamWrapper->initGraphics(w, h);
}

void setKeyboardOffset(int offset)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->setKeyboardOffset(offset);
    }
}

void keyPress(int key, bool down)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->keyPress(key,down!=0);
    }
}

void mouseEvent(int x,int y,int flags)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->mouseEvent(x,y,flags);
    }
}

void pause(bool pause)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->pause(pause!=0);
    }
}


// empty implementation for Apple clients
// since GLKViewController(ios) or GLView(osx) will repeatedly call
// step() via drawRect
void platformNewFrame()
{

}

// empty implementation for ios; no hardware decode yet
int platformBindVideoTexture( int textureID )
{
    return textureID;
}

void platformErrorMessage( bool fatal, const char * message )
{

    @autoreleasepool {
        NSMutableDictionary *userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:[NSString stringWithFormat:@"%s",message] forKey:kXSTX_CLIENT_STOP_MESSAGE];
        [userInfo setObject:[NSNumber numberWithBool:fatal] forKey:kXSTX_CLIENT_STOP_IS_FATAL];

        [[NSNotificationCenter defaultCenter]postNotificationName:kXSTX_CLIENT_STOPPED_NOTIFICATION object:nil userInfo:userInfo];
    }


}

/**
 * Let the platform know we've connected.
 */
void platformOnConnectSuccess() {
    @autoreleasepool {
        [[NSNotificationCenter defaultCenter]
         postNotificationName:kXSTX_CLIENT_READY_NOTIFICATION
         object:nil
         userInfo:nil];
    }

}


void platformOnStopped(int) {
    NSLog(@"%s", __PRETTY_FUNCTION__);
}

void platformOnReconnecting(uint32_t timeoutMs, const char *message)
{
    NSLog(@"%s %i %s", __PRETTY_FUNCTION__, timeoutMs, message);
    @autoreleasepool {
        NSMutableDictionary *userInfo = [NSMutableDictionary dictionary];
        [userInfo setObject:[NSString stringWithFormat:@"%s",message] forKey:kXSTX_RECONNECTING_MESSAGE];
        [userInfo setObject:[NSNumber numberWithInt:timeoutMs] forKey:kXSTX_RECONNECTING_TIMEOUT_IN_MS];
        
        [[NSNotificationCenter defaultCenter]
         postNotificationName:kXSTX_RECONNECTING_NOTIFICATION
         object:nil
         userInfo:userInfo];
    }
}

void platformOnReconnected()
{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    @autoreleasepool {
        [[NSNotificationCenter defaultCenter]
         postNotificationName:kXSTX_RECONNECTED_NOTIFICATION
         object:nil
         userInfo:nil];
    }
}
