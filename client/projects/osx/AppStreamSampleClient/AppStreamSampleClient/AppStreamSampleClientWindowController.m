
#import "constants.h"

#import "AppStreamSampleClientWindowController.h"
#import "AppStreamGLView.h"

#import "AppStreamClient.h"
#import "EntitlementRetriever.h"

#import "DESDialogWindowController.h"

#import "TextHelper.h"
#import "AppleTextHelper.h"

@interface AppStreamSampleClientWindowController ()
{
    EntitlementRetriever *mEntitlementRetreiver;
    DESDialogWindowController *mDESDialog;
    
    NSAlert *errorAlert;
    BOOL altIsDown;
    BOOL controlIsDown;
    BOOL shiftIsDown;
    BOOL windowsIsDown;
}

@end

@implementation AppStreamSampleClientWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (BOOL) acceptsFirstResponder
{
    return true;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // set up the GLView to render into
    BOOL success =  [self initializeGraphics];
    if(! success )
    {
        [self showError:@"Could not initialize open gl"];
    }

    //Create the AppStreamClient and tell it about the glView
    XStxResult initResult = [[AppStreamClient sharedClient] initialize:self.glView];
    
    if (initResult != XSTX_RESULT_OK)
    {
        [self showError:@"Failed to create XStx client"];
    }
    else
    {
        [self showStatus:@"Created XStx client"];
    }

    
    //Setup the mouse tracking
    int opts = (NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
    NSTrackingArea *area = [[NSTrackingArea alloc] initWithRect:[_glView bounds]
                                                        options:opts
                                                          owner:self
                                                       userInfo:nil];
    [_glView addTrackingArea:area];
    
    //Notification for when the window resizes
    [[NSNotificationCenter defaultCenter] addObserver:self selector:@selector(windowDidResize:) name:NSWindowDidResizeNotification object:self.window];
    
    [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowWillCloseNotification object:self.window queue:nil usingBlock:^(NSNotification *note) {
        //Make sure the DES dialog is closed too
        if (mDESDialog) {
            [mDESDialog close];
        }
    }];
    
    [[NSNotificationCenter defaultCenter] addObserverForName:NSWindowDidBecomeKeyNotification object:self.window queue:nil usingBlock:^(NSNotification *note) {
        //If the DESDialog is open make sure it stays the key window
        if (mDESDialog.window.isVisible) {
            [mDESDialog.window makeKeyWindow];
        }
    }];

    //Finish configuring the reconnecting view
    [self.reconnectingView setWantsLayer:true];
    self.reconnectingView.layer.backgroundColor = [NSColor colorWithRed:0 green:0 blue:0 alpha:.5].CGColor;
}

#pragma mark - Developer Entitlement Service dialog delegate
-(void) handleConnect:(NSDictionary *)userInfo
{
    NSNumber *num   = [userInfo valueForKey:@"standalonemode"];
    BOOL standaloneMode = [num boolValue];
    
    if(standaloneMode )
    {
        NSString *serverIP = [userInfo valueForKey:@"url"];
        
        //Pull sessionID and port from hostInfo
        NSString *filepath = [[NSBundle mainBundle]
                              pathForResource:@"HostInfo" ofType:@"plist"];
        NSDictionary *hostInfo = [NSDictionary
                                  dictionaryWithContentsOfFile:filepath];
        
        NSString *sessionID = [hostInfo valueForKey:@"XStxSessionId"];
        uint16_t serverPort = [[hostInfo valueForKey:@"StandaloneModePort"]integerValue];

        [mDESDialog reset];
        [mDESDialog dismiss];

        NSString *url = [NSString stringWithFormat:@"ssm://%@:%u?sessionId=%@",
                         serverIP, serverPort,sessionID];
        
        [self connectXStxClient: url];
    }
    else
    {
        // create entitlement retriever if necessary
        if (! mEntitlementRetreiver  )
        {
            mEntitlementRetreiver = [[EntitlementRetriever alloc] init];
            mEntitlementRetreiver.delegate = self;
        }
        
        //Make the request
        [mEntitlementRetreiver makeEntitlementsRequest:userInfo];
    }
}


-(void) connectXStxClient: (NSString*) entitlementUrl {
    if ([entitlementUrl length] <= 0) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showError:@"No entitlement url." ];
        });
        return;
    }
    
    [[AppStreamClient sharedClient] setDelegate:self];
    [[AppStreamClient sharedClient] connect:entitlementUrl];
}

#pragma mark - GLView Handling
-(BOOL) initializeGraphics
{
    //TODO: Make this match up with the incoming video size
    [self.window setContentAspectRatio:NSMakeSize(1280, 720)];
    //Create the AppStreamGLView
    self.glView = [[AppStreamGLView alloc] initWithFrame:((NSView*)self.window.contentView).frame];
    //Set the autoresizingMask properties
    self.glView.autoresizingMask = NSViewWidthSizable | NSViewHeightSizable;
    //Add it to the window
    [self.window.contentView addSubview:self.glView];
    [self.glView addSubview:_statusTextField];
    [self.glView addSubview:_fpsTextField];

    [self showFPS:@"0.0 FPS"];
    
    return true;
}


#pragma mark - Message Display
-(void) showDESDialog
{
    if (!mDESDialog) {
        mDESDialog = [[DESDialogWindowController alloc] initWithWindowNibName:@"DESDialogWindowController"];
        mDESDialog.delegate = self;
    }
    
    [mDESDialog showWindow:self];
    
    [self.window addChildWindow:mDESDialog.window ordered:NSWindowAbove];
}

-(void) showError: (NSString *) errorMessage
{
    //Make sure our error dialog is shown from the main thread
    if ([NSThread mainThread] != [NSThread currentThread])
    {
        [self performSelectorOnMainThread:@selector(showError:) withObject:errorMessage waitUntilDone:false];
        return;
    }
    
    NSLog(@"Error: %@", errorMessage);
    [self showStatus:errorMessage];
    
    [self showDESDialog];
    [mDESDialog setErrorText:errorMessage];
}

-(void) hideError
{
    [[errorAlert window] close];
    errorAlert = nil;
}

// log status of the connection to the screen via TextField
-(void) showStatus:(NSString*) message
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showStatus:message];
        });
        return;
    }
    
    [self.statusTextField setStringValue:message];
    NSString *msg = [NSString stringWithFormat:@"%@",message];
    NSLog(@"status: %@",msg);
    
    //Hide the message in a few seconds
    if (msg.length > 0) {
        //Cancel any clears we have scheduled
        [NSObject cancelPreviousPerformRequestsWithTarget:self selector:@selector(clearStatus) object:nil];
        
        //Schedule a clearStatus
        [self performSelector:@selector(clearStatus) withObject:nil afterDelay:5];
    }
}

-(void) clearStatus
{
    [self.statusTextField setStringValue:@""];
}

-(void) showFPS:(NSString*) fps
{
    [self.fpsTextField setStringValue: fps];
}


#pragma mark - AppStreamClientListenerDelegate methods
// these callback methods map to callback methods from the engine
-(void) clientReady;
{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    [self showStatus:@"XStx client ready"];
}
-(void) clientStopped:(NSString*) stopReason;
{
    NSLog(@"%s: %@", __PRETTY_FUNCTION__, stopReason);
    if (!self.isStopping)
    {
        [self showError:stopReason];
    }
}

-(void) clientReconnecting:(NSString *) reconnectingMessage withTimeoutInMS:(int) timeoutInMS;
{
    if (![NSThread isMainThread]) {
        //Since we are updating text we need to make sure it happens only on
        // the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            [self clientReconnecting:reconnectingMessage withTimeoutInMS:timeoutInMS];
        });
        return;
    }
    
    [self.reconnectingTextField setStringValue:[NSString stringWithFormat:@"Reconnecting: %@", reconnectingMessage]];
    self.reconnectingView.frame = self.glView.bounds;
    [self.glView addSubview:self.reconnectingView];
    [self showStatus:[NSString stringWithFormat:@"XStx attempting reconnect. Timeout: %is", (timeoutInMS/1000)]];
}

-(void) clientReconnected;
{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    [self.reconnectingView removeFromSuperview];
    [self showStatus:@"XStx client reconnected"];
}


// and this one can optionally be used to send info back to the platform
-(void) handleMessage:(NSString*) statusMessage;
{
    NSLog(@"%s: %@", __PRETTY_FUNCTION__, statusMessage);
    [self showStatus:statusMessage];
}

-(void) step:(float) fps;
{
    [self showFPS:[NSString stringWithFormat:@"%.2f FPS", fps]];
}


#pragma mark - Entitlement Retriever Delegate
-(void) didFailWithErrorMessage:(NSString*) errorMessage;
{
    NSLog(@"%s: %@", __PRETTY_FUNCTION__, errorMessage);
    [mDESDialog reset];
    [mDESDialog setErrorText:errorMessage];
}

// after retrieving entitlement url from entitlement service, use endpoint
// to connect to appstream service and retrieve a valid session
-(void) didRetreiveEntitlementUrl:(NSString *)entitlementUrl
{
    NSLog(@"%s %@", __PRETTY_FUNCTION__, entitlementUrl);
    [mDESDialog reset];
    [mDESDialog dismiss];
    
    [self showStatus:[NSString stringWithFormat:@"Requesting session from %@ ", entitlementUrl]];
    [self connectXStxClient:entitlementUrl];
}


#pragma mark - Input Handling
- (void) flagsChanged:(NSEvent *)theEvent
{
    //SHIFT
    if (([theEvent modifierFlags] & NSShiftKeyMask) == NSShiftKeyMask)
    {
        if (!shiftIsDown) {
            //Send the shift down event and set the shift flag
            [[AppStreamClient sharedClient] sendKeyDown:VK_SHIFT];
            shiftIsDown = true;
        }
    } else
    {
        if (shiftIsDown) {
            //Send the shift up event and reset the shift flag
            [[AppStreamClient sharedClient] sendKeyUp:VK_SHIFT];
            shiftIsDown = false;
        }
    }
    
    //CONTROL (Windows Key)
    // Likely a better user experience to treat CONTROL as WINDOWS KEY and COMMAND as CONTROL
    if (([theEvent modifierFlags] & NSControlKeyMask) == NSControlKeyMask) {
        if (!windowsIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_LWIN];
            windowsIsDown = true;
        }
    } else
    {
        if (windowsIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_LWIN];
            windowsIsDown = false;
        }
    }
    //COMMAND (Control Key)
    if (([theEvent modifierFlags] & NSCommandKeyMask) == NSCommandKeyMask) {
        if (!controlIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_CONTROL];
            controlIsDown = true;
        }
    } else
    {
        if (controlIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_CONTROL];
            controlIsDown = false;
        }
    }

    
    //ALT
    if (([theEvent modifierFlags] & NSAlternateKeyMask) == NSAlternateKeyMask) {
        if (!altIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_MENU];
            altIsDown = true;
        }
    } else
    {
        if (altIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_MENU];
            altIsDown = false;
        }
    }
}

-(int) getConvertedKeyUsingKeyCode: (NSEvent *) theEvent
{
    int keyCode = [theEvent keyCode];
    
    int newKey = [[AppStreamClient sharedClient] getVirtualKeyUsingKeyCode:keyCode];
    
    return newKey;
}

-(int) getConvertedKeyUsingCharacter: (NSEvent *) theEvent withModifierMask: (uint16_t &) modifierKeyMask withModifierKeyIgnoreMask: (uint16_t &) modifierKeysIgnoreMask
{
    //Get the characters from the key being pressed
    NSString *theKeyString = [theEvent charactersIgnoringModifiers];
    
    if ([theKeyString length] <= 0) {
        //Reject dead keys
        return -1;
    }
    
    //Get the characters as a char
    uint16_t theChar = [theKeyString characterAtIndex:0];
    
    //First check the generic key mapping, this should convert any non-system
    // specific keys
    bool didMapKey = getVirtualKeyUsingChar(theChar, modifierKeyMask, modifierKeysIgnoreMask);
    
    if (!didMapKey) {
        //The generic key mapping didn't map the key so check the platform specific ones
        didMapKey = appleGetVirtualKeyUsingChar(theChar, modifierKeyMask, modifierKeysIgnoreMask);
    }
    
    if (!didMapKey) {
        //Neither the generic nor the system specific mapping could remap the key
        NSLog(@"\n======== Unmapped KEY! =========");
        unichar upperKeyChar = [[theKeyString uppercaseString] characterAtIndex:0];
        NSString *modKeyString = [theEvent characters];

        NSLog(@"Key: %hu String: %@ Mod: %@", upperKeyChar, theKeyString, modKeyString);

        return -1;
    }
    
    return theChar;
}

-(void) processKeyEvent: (NSEvent *) theEvent
{
    //Depending on your application you might want to block repeat keys
    //    if ([theEvent isARepeat]) {
    //        //Ignore repeats
    //        return;
    //    }
    
    //Setup a couple bitmasks for the command keys
    uint16_t modifierKeyMask = 0;
    uint16_t modifierKeysIgnoreMask = 0;
    int newKey = [self getConvertedKeyUsingCharacter:theEvent withModifierMask:modifierKeyMask withModifierKeyIgnoreMask:modifierKeysIgnoreMask];

    if (newKey < 0)
    {
        //Not a valid key mapping just return
        return;
    }
    
    BOOL needAlt = (modifierKeyMask & STX_ALT_KEY_MASK) == STX_ALT_KEY_MASK;
    BOOL needControl = (modifierKeyMask & STX_CONTROL_KEY_MASK) == STX_CONTROL_KEY_MASK;
    BOOL needShift = (modifierKeyMask & STX_SHIFT_KEY_MASK) == STX_SHIFT_KEY_MASK;
    BOOL needWin = (modifierKeyMask & STX_WINDOWS_KEY_MASK) == STX_WINDOWS_KEY_MASK;
    
    //Check the status of the command keys
    if ((modifierKeysIgnoreMask & STX_SHIFT_KEY_MASK) != STX_SHIFT_KEY_MASK) {
        if (needShift && !shiftIsDown) {
            //Need shift pressed but it isn't
            [[AppStreamClient sharedClient] sendKeyDown:VK_SHIFT];
        } else if (!needShift && shiftIsDown)
        {
            //Need shift released but it is pressed
            [[AppStreamClient sharedClient] sendKeyUp:VK_SHIFT];
        }
    }
    
    if ((modifierKeysIgnoreMask & STX_ALT_KEY_MASK) != STX_ALT_KEY_MASK) {
        if (needAlt && !altIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_MENU];
        } else if (!needAlt && altIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyUp:VK_MENU];
        }
    }

    if ((modifierKeysIgnoreMask & STX_CONTROL_KEY_MASK) != STX_CONTROL_KEY_MASK) {
        if (needControl && !controlIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_CONTROL];
        } else if (!needControl && controlIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyUp:VK_CONTROL];
        }
    }

    if ((modifierKeysIgnoreMask & STX_WINDOWS_KEY_MASK) != STX_WINDOWS_KEY_MASK) {
        if (needWin && !windowsIsDown) {
            [[AppStreamClient sharedClient] sendKeyDown:VK_LWIN];
        } else if (!needWin && windowsIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyUp:VK_LWIN];
        }
    }

    //Now send the actual keydown event
    if (theEvent.type == NSKeyDown) {
        [[AppStreamClient sharedClient] sendKeyDown:newKey];
    } else
    {
        //Not keyDown so must be keyUp
        [[AppStreamClient sharedClient] sendKeyUp:newKey];
    }
    
    //Reset the command keys state
    if ((modifierKeysIgnoreMask & STX_SHIFT_KEY_MASK) != STX_SHIFT_KEY_MASK) {
        if (needShift && !shiftIsDown) {
            //Need shift pressed but it isn't
            [[AppStreamClient sharedClient] sendKeyUp:VK_SHIFT];
        } else if (!needShift && shiftIsDown)
        {
            //Need shift released but it is pressed
            [[AppStreamClient sharedClient] sendKeyDown:VK_SHIFT];
        }
    }
    
    if ((modifierKeysIgnoreMask & STX_ALT_KEY_MASK) != STX_ALT_KEY_MASK) {
        if (needAlt && !altIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_MENU];
        } else if (!needAlt && altIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyDown:VK_MENU];
        }
    }
    
    if ((modifierKeysIgnoreMask & STX_CONTROL_KEY_MASK) != STX_CONTROL_KEY_MASK) {
        if (needControl && !controlIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_CONTROL];
        } else if (!needControl && controlIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyDown:VK_CONTROL];
        }
    }
    
    if ((modifierKeysIgnoreMask & STX_WINDOWS_KEY_MASK) != STX_WINDOWS_KEY_MASK) {
        if (needWin && !windowsIsDown) {
            [[AppStreamClient sharedClient] sendKeyUp:VK_LWIN];
        } else if (!needWin && windowsIsDown)
        {
            [[AppStreamClient sharedClient] sendKeyDown:VK_LWIN];
        }
    }
}

- (void) keyDown:(NSEvent *)theEvent
{
    [self processKeyEvent: theEvent];
}

- (void) keyUp:(NSEvent *)theEvent
{
    [self processKeyEvent: theEvent];
}

- (void) mouseDown:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];
    
    theLoc.y = _glView.bounds.size.height - theLoc.y;

    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:RI_MOUSE_BUTTON_1_DOWN];
}

- (void) mouseUp:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];

    theLoc.y = _glView.bounds.size.height - theLoc.y;

    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:RI_MOUSE_BUTTON_1_UP];
}

- (void) mouseDragged:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];
    
    theLoc.y = _glView.bounds.size.height - theLoc.y;

    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:0];
}

- (void) rightMouseDown:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];
    
    theLoc.y = _glView.bounds.size.height - theLoc.y;
    
    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:RI_MOUSE_BUTTON_2_DOWN];
}

- (void) rightMouseUp:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];
    
    theLoc.y = _glView.bounds.size.height - theLoc.y;
    
    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:RI_MOUSE_BUTTON_2_UP];
}

- (void) rightMouseDragged:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];
    
    theLoc.y = _glView.bounds.size.height - theLoc.y;
    
    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:0];
}

- (void) mouseMoved:(NSEvent *)theEvent
{
    NSPoint theLoc = [theEvent locationInWindow];

    theLoc.y = _glView.bounds.size.height - theLoc.y;

    [[AppStreamClient sharedClient] sendMouseEvent:theLoc flags:0];
}


///* The following two message are the preferred API for accessing NSScrollWheel deltas. When -hasPreciseScrollDeltas reutrns NO, multiply the returned value by line or row height. When -hasPreciseScrollDeltas returns YES, scroll by the returned value (in points).
// */
- (void) scrollWheel:(NSEvent *)theEvent
{
    float scrollMultiplier = 15.0; //lineHeight
    if (theEvent.hasPreciseScrollingDeltas) {
        scrollMultiplier = 1.0;
    }
    
    //x scrolling is ignored so put the y change into the data we pass to the server
//    float xDiff = theEvent.scrollingDeltaX * scrollMultiplier;
    float yDiff = theEvent.scrollingDeltaY * scrollMultiplier;
    CGPoint pt = CGPointMake(yDiff, 0);
    
    [[AppStreamClient sharedClient] sendMouseEvent:pt flags:CET_MOUSE_WHEEL];
}

#pragma mark - Window Handling
-(void) windowDidResize: (NSNotification *) notification
{
    [[AppStreamClient sharedClient] updateDimensions: self.glView.frame.size];
}

@end
