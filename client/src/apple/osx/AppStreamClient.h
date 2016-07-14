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
 * AppStreamClient - singleton class for abstracting xstx module
 * and providing basic objective-c api to start XStx client
 *
 * Provides wrapper methods for initializing, connecting and inputs
 * Become an AppStreamClientListenerDelegate and receive client stop messages
 * in the main view controller
 */


#import <Foundation/Foundation.h>
#import <GLKit/GLKit.h>
#import "XStx/client/XStxClientAPI.h"
#import "AppStreamWrapper.h"


@protocol AppStreamClientListenerDelegate <NSObject>

@optional
// these callback methods map to callback methods from the engine
-(void) clientReady;
-(void) clientStopped:(NSString*) stopReason;
-(void) clientReconnecting:(NSString *) reconnectingMessage withTimeoutInMS:(int) timeoutInMS;
-(void) clientReconnected;

// and this one can optionally be used to send info back to the platform
-(void) handleMessage:(NSString*) statusMessage;
-(void) step:(float) fps;

@end


@interface AppStreamClient : NSObject

+(AppStreamClient*) sharedClient;
-(XStxResult) initialize:(NSOpenGLView *)glview;
-(XStxResult) connect:(NSString *) entitlementUrl;
-(void) stop;
-(XStxResult) recycle;

-(void) sendInput:(XStxInputEvent )event;
-(void) sendRawInput:(NSString *)inputString;

-(void) sendKeyDown:(int) key;
-(void) sendKeyUp:(int) key;
//// getVirtualKey -- get a Virtual key code
-(uint32_t) getVirtualKeyUsingKeyCode:(int) keyCode;
-(void) sendKeyEvent:(int)key isDown:(BOOL) down;
-(void) sendMouseEvent:(CGPoint ) xy flags:(uint32_t)flags;

-(void) renderFrame;

-(void) updateDimensions: (CGSize) newSize;

@property(nonatomic, weak) id delegate;

@end


