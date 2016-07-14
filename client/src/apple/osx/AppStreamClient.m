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

//Import Carbon.h so we can get to HIToolbox/Events.h for keycode mapping
#import <Carbon/Carbon.h>

#import "constants.h"

#import "TextHelper.h"
#import "AppleTextHelper.h"

// Global reference to an xstx module
AppStreamWrapper *gAppStreamWrapper = NULL;
static AppStreamClient *sharedClient;

@interface AppStreamClient()

@end

@implementation AppStreamClient

+  (AppStreamClient*) sharedClient
{
    static dispatch_once_t singleton_predicate;
    dispatch_once(&singleton_predicate, ^{
        sharedClient = [[AppStreamClient alloc] init];
    });

    return sharedClient;
}

- (AppStreamClient *)init
{
    self = [super init];
    if (self) {
        [self createAppStreamWrapper];
        
        __weak AppStreamClient *unretainedSelf = self;
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_CLIENT_STOPPED_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *notification) {
            
            if( [unretainedSelf.delegate respondsToSelector:@selector(clientStopped:)] )
            {
                [unretainedSelf.delegate clientStopped:[notification.userInfo objectForKey:kXSTX_CLIENT_STOP_MESSAGE]];
            }
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_CLIENT_READY_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *note) {
            
            if( [unretainedSelf.delegate respondsToSelector:@selector(clientReady)] )
            {
                [unretainedSelf.delegate clientReady];
            }
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_RECONNECTING_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *note) {

            if( [unretainedSelf.delegate respondsToSelector:@selector(clientReconnecting:withTimeoutInMS:)] )
            {
                [unretainedSelf.delegate clientReconnecting:[note.userInfo objectForKey: kXSTX_RECONNECTING_MESSAGE] withTimeoutInMS:[[note.userInfo objectForKey:kXSTX_RECONNECTING_TIMEOUT_IN_MS] intValue]];
            }
        }];
        
        [[NSNotificationCenter defaultCenter] addObserverForName:kXSTX_RECONNECTED_NOTIFICATION object:nil queue:nil usingBlock:^(NSNotification *note) {

            if ([unretainedSelf.delegate respondsToSelector:@selector(clientReconnected)]) {
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

-(void) renderFrame
{
    if (gAppStreamWrapper)
    {
        if (gAppStreamWrapper->getVideoRenderer()) {
            gAppStreamWrapper->step();
        }
        
        if( [self.delegate respondsToSelector:@selector(step:)])
        {
            [self.delegate step:gAppStreamWrapper->getFPS()];
        }
    }
}

-(XStxResult) initialize:(NSOpenGLView *)glview
{
    //Only init the AppStreamWrapper once
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        gAppStreamWrapper->init();
    });

    CGFloat frameWidth = glview.frame.size.width;
    CGFloat frameHeight = glview.frame.size.height;

    gAppStreamWrapper->initGraphics(frameWidth, frameHeight);
    
    id <AppStreamClientListenerDelegate> delegate = [AppStreamClient sharedClient].delegate;
    if( [delegate respondsToSelector:@selector(handleMessage:)])
    {
        [delegate handleMessage:@"AppStream Client initialized"];
    }

    return XSTX_RESULT_OK;

}

-(XStxResult) connect:(NSString *) entitlementUrl
{
    NSLog(@"Connecting to URL: %@", entitlementUrl);
    return gAppStreamWrapper->connect([entitlementUrl UTF8String]);
}

-(void) stop
{
    return gAppStreamWrapper->stop();
}

-(XStxResult) recycle
{
    return gAppStreamWrapper->recycle();
}

-(void) sendInput:(XStxInputEvent )event
{
    gAppStreamWrapper->sendInput(event);
}

-(void) sendRawInput:(NSString *)inputString
{
    gAppStreamWrapper->sendRawInput((const uint8_t*)[inputString UTF8String], (int)inputString.length);
}



#pragma mark - Keyboard event handling
/// convenience methods for up and down key presses
-(void) sendKeyDown:(int) key
{
    [self sendKeyEvent:key isDown: true];
}

-(void) sendKeyUp:(int) key
{
    [self sendKeyEvent:key isDown: false];
}

///
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
    NSString *upordown = (down) ? @"down" : @"up";
//    NSLog(@"sending key %@ event for key %d", upordown,key);
}

//// getVirtualKey -- get a Virtual key code
//Mac keycodes map to the ANSI position on a keyboard
// thus kVK_ANSI_A maps to the key in the A position
// on a US keyboard. In other layouts that will be a different letter
// for WASD style mappings this is probably the way to go but for text entry
// it will not work right for non-US keyboards
-(uint32_t) getVirtualKeyUsingKeyCode:(int)keyCode
{
    switch (keyCode) {
        case kVK_ANSI_A:
            return kKEY_A;
            break;

        case kVK_ANSI_S:
            return kKEY_S;
            break;

        case kVK_ANSI_D:
            return kKEY_D;
            break;

        case kVK_ANSI_F:
            return kKEY_F;
            break;

        case kVK_ANSI_H:
            return kKEY_H;
            break;

        case kVK_ANSI_G:
            return kKEY_G;
            break;
        case kVK_ANSI_Z:
            return kKEY_Z;
            break;

        case kVK_ANSI_X:
            return kKEY_X;
            break;

        case kVK_ANSI_C:
            return kKEY_C;
            break;

        case kVK_ANSI_V:
            return kKEY_V;
            break;

        case kVK_ANSI_B:
            return kKEY_B;
            break;

        case kVK_ANSI_Q:
            return kKEY_Q;
            break;

        case kVK_ANSI_W:
            return kKEY_W;
            break;

        case kVK_ANSI_E:
            return kKEY_E;
            break;

        case kVK_ANSI_R:
            return kKEY_R;
            break;

        case kVK_ANSI_Y:
            return kKEY_Y;
            break;

        case kVK_ANSI_T:
            return kKEY_T;
            break;

        case kVK_ANSI_1:
            return kKEY_1;
            break;

        case kVK_ANSI_2:
            return kKEY_2;
            break;

        case kVK_ANSI_3:
            return kKEY_3;
            break;

        case kVK_ANSI_4:
            return kKEY_4;
            break;

        case kVK_ANSI_6:
            return kKEY_6;
            break;

        case kVK_ANSI_5:
            return kKEY_5;
            break;

        case kVK_ANSI_Equal:
            return VK_OEM_PLUS;
            break;

        case kVK_ANSI_9:
            return kKEY_9;
            break;

        case kVK_ANSI_7:
            return kKEY_7;
            break;

        case kVK_ANSI_Minus:
            return VK_OEM_MINUS;
            break;

        case kVK_ANSI_8:
            return kKEY_8;
            break;

        case kVK_ANSI_0:
            return kKEY_0;
            break;

        case kVK_ANSI_RightBracket:
            return VK_OEM_6;
            break;

        case kVK_ANSI_O:
            return kKEY_O;
            break;

        case kVK_ANSI_U:
            return kKEY_U;
            break;

        case kVK_ANSI_LeftBracket:
            return VK_OEM_4;
            break;

        case kVK_ANSI_I:
            return kKEY_I;
            break;

        case kVK_ANSI_P:
            return kKEY_P;
            break;

        case kVK_ANSI_L:
            return kKEY_L;
            break;

        case kVK_ANSI_J:
            return kKEY_J;
            break;

        case kVK_ANSI_Quote:
            return VK_OEM_7;
            break;

        case kVK_ANSI_K:
            return kKEY_K;
            break;

        case kVK_ANSI_Semicolon:
            return VK_OEM_1;
            break;

        case kVK_ANSI_Backslash:
            return VK_OEM_5;
            break;

        case kVK_ANSI_Comma:
            return VK_OEM_COMMA;
            break;

        case kVK_ANSI_Slash:
            return VK_OEM_2;
            break;

        case kVK_ANSI_N:
            return kKEY_N;
            break;

        case kVK_ANSI_M:
            return kKEY_M;
            break;

        case kVK_ANSI_Period:
            return VK_OEM_PERIOD;
            break;

        case kVK_ANSI_Grave:
            return VK_OEM_3;
            break;

        case kVK_ANSI_KeypadDecimal:
            return VK_DECIMAL;
            break;

        case kVK_ANSI_KeypadMultiply:
            return VK_MULTIPLY;
            break;

        case kVK_ANSI_KeypadPlus:
            return VK_ADD;
            break;

        case kVK_ANSI_KeypadClear:
            return VK_OEM_CLEAR;
            break;

        case kVK_ANSI_KeypadDivide:
            return VK_DIVIDE;
            break;

        case kVK_ANSI_KeypadEnter:
            return VK_RETURN;
            break;

        case kVK_ANSI_KeypadMinus:
            return VK_SUBTRACT;
            break;

        case kVK_ANSI_KeypadEquals:
            return VK_OEM_PLUS;
            break;

        case kVK_ANSI_Keypad0:
            return VK_NUMPAD0;
            break;

        case kVK_ANSI_Keypad1:
            return VK_NUMPAD1;
            break;

        case kVK_ANSI_Keypad2:
            return VK_NUMPAD2;
            break;

        case kVK_ANSI_Keypad3:
            return VK_NUMPAD3;
            break;

        case kVK_ANSI_Keypad4:
            return VK_NUMPAD4;
            break;

        case kVK_ANSI_Keypad5:
            return VK_NUMPAD5;
            break;

        case kVK_ANSI_Keypad6:
            return VK_NUMPAD6;
            break;

        case kVK_ANSI_Keypad7:
            return VK_NUMPAD7;
            break;

        case kVK_ANSI_Keypad8:
            return VK_NUMPAD8;
            break;

        case kVK_ANSI_Keypad9:
            return VK_NUMPAD9;
            break;


            //These keys are independent of hardware layouts
        case kVK_Return:
            return VK_RETURN;
            break;

        case kVK_Tab:
            return VK_TAB;
            break;

        case kVK_Space:
            return VK_SPACE;
            break;

        case kVK_Delete:
            return VK_BACK;
            break;

        case kVK_Escape:
            return VK_ESCAPE;
            break;

            //Better user experience to treat Command as Control and
            // control as Windows key
        case kVK_Command:
            return VK_CONTROL;
            break;

        case kVK_Control:
            return VK_LWIN;
            break;

        case kVK_RightControl:
            return VK_LWIN;
            break;

        case kVK_Shift:
            return VK_SHIFT;
            break;

        case kVK_CapsLock:
            return VK_CAPITAL;
            break;

        case kVK_Option:
            return VK_MENU;
            break;

        case kVK_RightShift:
            return VK_RSHIFT;
            break;

        case kVK_RightOption:
            return VK_RMENU;
            break;

        case kVK_F17:
            return VK_F17;
            break;

        case kVK_VolumeUp:
            return VK_VOLUME_UP;
            break;

        case kVK_VolumeDown:
            return VK_VOLUME_DOWN;
            break;

        case kVK_Mute:
            return VK_VOLUME_MUTE;
            break;

        case kVK_F18:
            return VK_F18;
            break;

        case kVK_F19:
            return VK_F19;
            break;

        case kVK_F20:
            return VK_F20;
            break;

        case kVK_F5:
            return VK_F5;
            break;

        case kVK_F6:
            return VK_F6;
            break;

        case kVK_F7:
            return VK_F7;
            break;

        case kVK_F3:
            return VK_F3;
            break;

        case kVK_F8:
            return VK_F8;
            break;

        case kVK_F9:
            return VK_F9;
            break;

        case kVK_F11:
            return VK_F11;
            break;

        case kVK_F13:
            return VK_F13;
            break;

        case kVK_F16:
            return VK_F16;
            break;

        case kVK_F14:
            return VK_F14;
            break;

        case kVK_F10:
            return VK_F10;
            break;

        case kVK_F12:
            return VK_F12;
            break;

        case kVK_F15:
            return VK_F15;
            break;

        case kVK_Help:
            return VK_HELP;
            break;

        case kVK_Home:
            return VK_HOME;
            break;

        case kVK_PageUp:
            return VK_PRIOR;
            break;

        case kVK_ForwardDelete:
            return VK_DELETE;
            break;

        case kVK_F4:
            return VK_F4;
            break;

        case kVK_End:
            return VK_END;
            break;

        case kVK_F2:
            return VK_F2;
            break;

        case kVK_PageDown:
            return VK_NEXT;
            break;

        case kVK_F1:
            return VK_F1;
            break;

        case kVK_LeftArrow:
            return VK_LEFT;
            break;

        case kVK_RightArrow:
            return VK_RIGHT;
            break;

        case kVK_DownArrow:
            return VK_DOWN;
            break;

        case kVK_UpArrow:
            return VK_UP;
            break;

        case kVK_Function:
        default:
            NSLog(@"No key mapping for key: %i", keyCode);
            break;
    }


    return -1;


}




// fill out XStxInputEvent struct emulating win32 RAWMOUSE
-(void) sendMouseEvent:(CGPoint ) xy flags:(uint32_t)flags
{
    gAppStreamWrapper->mouseEvent(xy.x, xy.y, flags);
}

-(void) updateDimensions:(CGSize)newSize
{
    if (gAppStreamWrapper->getVideoRenderer()) {
        gAppStreamWrapper->getVideoRenderer()->setDisplayDimensions(newSize.width, newSize.height);
    }
}

-(void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self];
}



@end
