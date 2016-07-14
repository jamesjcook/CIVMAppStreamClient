
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


#import "AppStreamClient.h"

#import "constants.h"

// UI view used for onscreen status messages
#import "MessageView.h"

#import "UIView+fade.h"
#import "UIApplication+views.h"


// Global reference to an xstx module
AppStreamWrapper *gAppStreamWrapper = NULL;
static AppStreamClient *sharedClient;


@interface AppStreamClient()
{
    MessageView *mMessageView;
    NSTimer *mMessageTimer;

}

-(uint32_t) getVirtualKey:(int) keyCode;

@end

@implementation AppStreamClient

+ (AppStreamClient *) sharedClient
{
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedClient = [[AppStreamClient alloc] init];
    });
    
    return sharedClient;
}

- (AppStreamClient *) init
{
    self = [super init];
    if (self) {
        [self createAppStreamWrapper];
        
        __weak AppStreamClient *unretainedSelf = self;
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_CLIENT_STOPPED_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *notification) {
            
            if ([unretainedSelf.delegate respondsToSelector:@selector(clientStopped:)])
            {
                [unretainedSelf.delegate clientStopped:[notification.userInfo valueForKey:kXSTX_CLIENT_STOP_MESSAGE]];
            }
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:UIApplicationWillTerminateNotification object:nil queue:nil usingBlock:^(NSNotification *notification) {
            
            [unretainedSelf recycle];
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_USER_STOP_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *note) {
            [unretainedSelf stop];
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_CLIENT_READY_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *note) {
            
            if( [unretainedSelf.delegate respondsToSelector:@selector(clientReady)] )
            {
                [unretainedSelf.delegate clientReady];
            }
            
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_RECONNECTING_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *notification) {
            
            if ([unretainedSelf.delegate respondsToSelector:@selector(clientReconnecting:withTimeoutInMS:)])
            {
                [unretainedSelf.delegate clientReconnecting:[notification.userInfo objectForKey:kXSTX_RECONNECTING_MESSAGE] withTimeoutInMS:[[notification.userInfo objectForKey:kXSTX_RECONNECTING_TIMEOUT_IN_MS] intValue]];
            }
        }];

        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_RECONNECTED_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *notification) {
            
            if ([unretainedSelf.delegate respondsToSelector:@selector(clientReconnected)])
            {
                [unretainedSelf.delegate clientReconnected];
            }
        }];

    }
    return self;
}

-(void) createAppStreamWrapper
{
    if (gAppStreamWrapper == NULL)
    {
        gAppStreamWrapper = new AppStreamWrapper();
    }
}

-(void) stop
{
    return gAppStreamWrapper->stop();
    [EAGLContext setCurrentContext:nil];
}


-(XStxResult) initialize:(GLKView *)glkview
{
    
    static dispatch_once_t onceToken;
    
    // Only ever initialize the XStx client once
    dispatch_once (&onceToken, ^{
        gAppStreamWrapper->init();
    });
    
    [EAGLContext setCurrentContext:glkview.context];
    _mGLKView = glkview;
    _mGLKView.delegate = self;
    
    // flipped for landscape
    CGFloat frameWidth = glkview.bounds.size.height*[UIScreen mainScreen].scale;
    CGFloat frameHeight = glkview.bounds.size.width*[UIScreen mainScreen].scale;
    
    gAppStreamWrapper->initGraphics(frameHeight, frameWidth, true);
    
    if( [self.delegate respondsToSelector:@selector(handleMessage:)])
    {
        [self.delegate handleMessage:@"AppStream Client initialized"];
    }
    
    return XSTX_RESULT_OK;
    
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    gAppStreamWrapper->step();
    
    if( [self.delegate respondsToSelector:@selector(step:)])
    {
        [self.delegate step:gAppStreamWrapper->getFPS()];
    }
}


-(XStxResult) connect:(NSString *) entitlementUrl
{
    return gAppStreamWrapper->connect([entitlementUrl UTF8String]);
}

-(XStxResult) recycle
{
    return gAppStreamWrapper->recycle();
}

-(void) sendInput:(XStxInputEvent )event
{
    gAppStreamWrapper->sendInput(event);
}

-(void) sendMessage:( const unsigned char *) message length:( uint32_t) length
{
    gAppStreamWrapper->sendMessage(message, length);
}

#pragma mark - Input event handling
// send a key press using ascii code
-(void) sendKey:(int)key {

    int newkey = [self getVirtualKey:key];

    // in some cases it is necessary to send a shift key in addition
    if ( ( newkey >= 'A' && newkey <= 'Z' ) || ( newkey == kUNDERSCORE ) )
    {
        [self sendShiftedKey:newkey];
    }
    else
    {
        [self sendKeyDown:toupper( newkey)];
        [self sendKeyUp:toupper(newkey)];
    }
}

/// convenience methods for up and down key presses
-(void) sendKeyDown:(int) key
{
    [self sendKeyEvent:key isDown: true];
}

-(void) sendKeyUp:(int) key
{
    [self sendKeyEvent:key isDown: false];
}

//
-(void) sendKeyEvent:(int)key isDown:(BOOL) down
{
    XStxInputEvent xstxKeyEvent = {0};
    xstxKeyEvent.mTimestampUs = CACurrentMediaTime() * 1000000.0;
    xstxKeyEvent.mDeviceId = 0;
    xstxKeyEvent.mUserId = 0;
    xstxKeyEvent.mType = XSTX_INPUT_EVENT_TYPE_KEYBOARD;
    xstxKeyEvent.mInfo.mKeyboard.mIsKeyDown = down;
    xstxKeyEvent.mInfo.mKeyboard.mVirtualKey = (int32_t) key;
    xstxKeyEvent.mSize = sizeof(XStxInputEvent);
    [self sendInput:xstxKeyEvent];
}

/// construct the sequence to send a shifted, such as '_'
-(void) sendShiftedKey:(int) key
{
    [self sendKeyDown:kSHIFT];
    [self sendKeyDown:key];
    [self sendKeyUp:key];
    [self sendKeyUp:kSHIFT];
}


//// getVirtualKey -- get a Virtual key code
-(uint32_t) getVirtualKey:(int) keyCode
{
    switch (keyCode)
    {
        case 44: // comma
            return kCOMMA;
            break;
        case 45: // minus
            return kMINUS;
            break;
        case 46: // period
            return kPERIOD;
            break;
        case 47: // forward slash
            return kFSLASH;
            break;
        case 59: // semicolon
            return kSEMICOLON;
            break;
        case 95: // underscore
            return kUNDERSCORE;
            break;
        case 126: // tilde
            return kTILDE;
            break;
        default:
            break;
    }

    return keyCode;
}

-(void) setKeyboardOffset:(int)offset
{
    gAppStreamWrapper->setKeyboardOffset(offset);
}

// fill out XStxInputEvent struct emulating win32 RAWMOUSE
-(void) sendMouseEvent:(CGPoint ) xy flags:(uint32_t)flags
{
    //Scale the xy by the screen scale to account for retina displays
    xy = CGPointMake(xy.x * [UIScreen mainScreen].scale, xy.y * [UIScreen mainScreen].scale);

    gAppStreamWrapper->mouseEvent(xy.x, xy.y, flags);
}

#pragma mark - Message handling
-(void) showMessageWithText:(NSString*)text
{
    if(mMessageView == nil)
    {
        CGRect frame = CGRectMake(10, 10, 420, 24);
        mMessageView = [[MessageView alloc]initWithFrame:frame andMessage:text];
        [mMessageView hide];
        [[UIApplication topView] addSubview:mMessageView];
        [mMessageView fadeIn];
        mMessageView.alpha = 1;

    }
    else
    {
        [mMessageView setMessage:text];
    }

    if ( [mMessageTimer isValid])
    {
        [mMessageTimer invalidate];
    }

    mMessageTimer = [NSTimer scheduledTimerWithTimeInterval:2.0 target:self selector:@selector(hideMessage:) userInfo:Nil repeats:NO];
}

-(void) hideMessage:(NSTimer*) timer
{
    [mMessageView fadeOutAndRemove];
    mMessageView = nil;
}



-(void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
    // clean up global gAppStreamWrapper;
    delete gAppStreamWrapper;
}



@end
