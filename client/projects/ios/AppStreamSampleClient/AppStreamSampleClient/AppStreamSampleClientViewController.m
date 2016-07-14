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

//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//\\//

#import <GLKit/GLKit.h>
#import "AppStreamSampleClientViewController.h"
#import "UIView+fade.h"
#import "constants.h"


@interface AppStreamSampleClientViewController()<UITextFieldDelegate,UIGestureRecognizerDelegate>
{
    // Developer Entitlement Service (DES) dialog
    DESDialog *mDESDialog;
    EntitlementRetriever *mEntitlementRetreiver;
}


// opengl context to render into
@property EAGLContext *oglContext;
// GLKview that will be set as this viewcontroller's view
@property (nonatomic) GLKView *glkview;
// child view controller for opengl rendering layer
@property (nonatomic,strong) GLKViewController *childGLKViewController;

/// Connect to a server using the entitlementUrl
-(void) connectXStxClient: (NSString*) entitlementUrl ;
/// print an XStxResult's name and description with optional message to console
- (void) printResult:(XStxResult) result withMessage:(NSString *) message;
/// show a status message onscreen
- (void) showStatus:(NSString*) message;

//Shows an error "dialog" on the screen
-(void) showError:(NSString*) message;

// show the Developer Entitlement Service dialog
-(void) showDESDialog:(id) sender;
@end


@implementation AppStreamSampleClientViewController


#pragma mark - View lifecycle

- (void)viewDidLoad
{
    [super viewDidLoad];
}

-(void) viewDidAppear:(BOOL)animated
{
    // set up the GLKView to render into
    BOOL success =  [self initializeGraphics];
    
    [self showDESDialog:nil];

    if(! success )
    {
        [self showError:@"Could not initialize OpenGL"];
    }
    
    XStxResult initResult = [[AppStreamClient sharedClient] initialize:self.glkview];
    
    if (initResult != XSTX_RESULT_OK)
    {
        [self showError:@"Failed to create XStx client"];
    }
    else
    {
        [self showStatus:@"Created XStx client"];
    }
}

- (void)viewDidUnload {
    [self setStatusLabel:nil];
    [super viewDidUnload];
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
}

-(BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
    return UIInterfaceOrientationIsLandscape(toInterfaceOrientation);
}

-(NSUInteger)supportedInterfaceOrientations
{
    // allows upsidedown orientation for iphone
    return UIInterfaceOrientationMaskAll;
}

// clean up
-(void) dealloc {
    [[AppStreamClient sharedClient] recycle];
}


#pragma mark - Client creation methods

-(BOOL) initializeGraphics {
    static BOOL didSetupGLView = NO;
    
    if( didSetupGLView )
    {
        return TRUE;
    }
    
    // Create an OpenGL Context to render into
    self.oglContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    if (!self.oglContext )
    {
        [self showStatus:@"Failed to create ES context"];
        return false;
    }
    
    
    CGRect glframe = self.view.bounds;
    
    if ([UIDevice currentDevice].userInterfaceIdiom == UIUserInterfaceIdiomPhone)
    {
        glframe = CGRectMake(0,0,self.view.frame.size.height,self.view.frame.size.width);
    }
    
    self.glkview = [[GLKView alloc] initWithFrame:glframe context:self.oglContext];
    self.glkview.multipleTouchEnabled = true;
    self.childGLKViewController = [[GLKViewController alloc] init];
    self.childGLKViewController.view.frame = glframe;
    self.childGLKViewController.view = self.glkview;

    [self.view addSubview:self.childGLKViewController.view];
    
    
    if(! self.glkview)
    {
        [self showStatus:@"Failed to create GLKView "];
        return false;
    }
    
    // set desired frames per second on the GLKViewController
    if(self.glkview != nil){
        didSetupGLView = YES;
        self.childGLKViewController.preferredFramesPerSecond = 60.0f;
        [self addChildViewController:(UIViewController*) self.childGLKViewController];
        
        //Make sure these views are in the front
        [self.view addSubview:_statusLabel];
        [self.view addSubview:_fpsLabel];
        [self.view addSubview:_keyboardButton];
        [self.view addSubview:_stopButton];
        [self.view addSubview:_backgroundImageView];
    }
    
    [self showFPS:@"0.0 FPS"];
    
    return didSetupGLView;
}


-(void) connectXStxClient: (NSString*) entitlementUrl {
    if ([entitlementUrl length] <= 0) {
        [self showError:@"No entitlement url." ];
        return;
    }
    
    [[AppStreamClient sharedClient] connect:entitlementUrl];
    [[AppStreamClient sharedClient] setDelegate:self];
    
    [self hideBackground];
}



#pragma mark - UITextField delegate
// for intercepting keyboard events
- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string {
    if( string.length > 0){
        [[AppStreamClient sharedClient] sendKey:(int)[string characterAtIndex:0]];
    }
    return NO;
}

- (void)textFieldDidDelete {
    [[AppStreamClient sharedClient] sendKey:kBACK]; // windows VK_BACK BACKSPACE key
}

- (BOOL) textFieldShouldReturn:(UITextField *)textField
{
    [[AppStreamClient sharedClient] sendKey:kENTER];
    return NO;
}

#pragma mark - Utility methods

-(IBAction) toggleKeyboard:(UIButton*)sender
{
    if([_inputTextField isFirstResponder])
    {
        [_inputTextField resignFirstResponder];
        [self showStatus:@""];
    }
    else
    {
        _inputTextField.deletekey_delegate = self;
        [_inputTextField becomeFirstResponder];
        [self showStatus:@"keyboard input"];
    }
}

- (IBAction)pressedStop:(id)sender {
    //User wants to stop the stream
    [[NSNotificationCenter defaultCenter] postNotificationName:kXSTX_USER_STOP_NOTIFICATION object:self userInfo:nil];
}

#pragma mark - Touch Handling
-(void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    [[AppStreamClient sharedClient] sendMouseEvent:[touch locationInView:self.view] flags:RI_MOUSE_BUTTON_1_DOWN];
}

-(void) touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    //Handle touchesCancelled exactly like touchesEnded
    [self touchesEnded:touches withEvent:event];
}

-(void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    [[AppStreamClient sharedClient] sendMouseEvent:[touch locationInView:self.view] flags:RI_MOUSE_BUTTON_1_UP];
    
    // clear status
    [self showStatus:@""];
    
}

-(void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    UITouch *touch = [touches anyObject];
    [[AppStreamClient sharedClient] sendMouseEvent:[touch locationInView:self.view] flags:0];
}

#pragma mark - Logging methods
// log result to console with description
- (void) printResult:(XStxResult) result withMessage:(NSString *) message
{
    const char * name; const char * desc;
    XStxResultGetInfo(result, &name, &desc);
    NSString *statusMessage = [NSString stringWithFormat:@"%@ result: %s (%s)\n",  message,name,desc];
    NSLog(@"printResult: %@",statusMessage);
    if(result != XSTX_RESULT_OK)
    {
        [self showError:statusMessage];
    }
    else
    {
        [self showStatus:statusMessage ];
    }
}

// log status of the connection to the screen via label
-(void) showStatus:(NSString*) message
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showStatus:message];
        });
        return;
    }
    NSString *msg = [NSString stringWithFormat:@"%@",message];
    _statusLabel.text = msg;
    
}


// display an error
-(void) showError:(NSString*) message
{
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showError:message];
        });
        return;
    }
    [self showDESDialog:self];
    [mDESDialog setErrorText:message];
}

#pragma mark - Misc. view handling
-(void) hideBackground
{
    [_backgroundImageView fadeOutAndRemove];
}

// present a dialog to collect connection info
-(void) showDESDialog:(id) sender
{
    if (![_backgroundImageView isDescendantOfView:self.view]) {
        [self.view addSubview:_backgroundImageView];
        [_backgroundImageView fadeIn];
    }
    
    if(! mDESDialog )
    {
        mDESDialog = [[DESDialog alloc]initWithFrame:self.view.bounds];
        mDESDialog.delegate = self;
    }
    if ( ![mDESDialog isDescendantOfView:self.view] )
    {
        [self.view addSubviewWithFade:mDESDialog];
        [mDESDialog reset];
        [self showStatus:@""];
    }
}

-(void) showFPS:(NSString*) fps
{
    _fpsLabel.text = fps;
}


#pragma mark - GLViewManagerDelegate -- for fps metrics
// calculate a simple moving average and display on screen
-(void) didRenderFrame:(double) finishTime
{
    static bool firstRun = false;
    
    if(! firstRun )
    {
        [self showStatus:@"stream data received."];
        double delayInSeconds = 2.0f;
        dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
        dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
            [self showStatus:@""];
        });
        firstRun = true;
        
        if([mDESDialog isDescendantOfView:self.view])
        {
            // use dismiss method so dialog can resignFirstResponder
            [mDESDialog dismiss];
        }
        
    }
    
};

#pragma mark - DES dialog delegate
-(void) handleConnect:(NSDictionary *)userInfo
{
    BOOL standaloneMode = [[userInfo valueForKey:@"standalonemode"] boolValue];
    
    if(standaloneMode )
    {
        //Get the address from user input
        NSString *serverAddress = [userInfo valueForKey:@"url"];
        
        //Pull sessionID and port from HostInfo.plist
        NSString *filepath = [[NSBundle mainBundle]
                              pathForResource:@"HostInfo" ofType:@"plist"];
        NSDictionary *hostInfo = [NSDictionary
                                  dictionaryWithContentsOfFile:filepath];
        
        NSString *sessionID = [hostInfo valueForKey:@"XStxSessionId"];
        int port  = [[hostInfo valueForKey:@"StandaloneModePort"] intValue];
        
        //We want to add ssm:// to the beginning if it isn't already there
        NSString *urlPrependText = @"ssm://";
        if ([serverAddress hasPrefix:urlPrependText]) {
            urlPrependText = @"";
        }
        
        //Craft the final url we will connect with
        NSString *url = [NSString stringWithFormat:@"%@%@:%u?sessionId=%@",
                         urlPrependText, serverAddress, port,sessionID];
        
        //Dismiss the DES dialog before we start the connect
        [mDESDialog reset];
        [mDESDialog dismiss];
        
        //Start the connect sequence
        [self connectXStxClient: url];
    }
    else
    {
        // create entitlement retriever
        if (! mEntitlementRetreiver  )
        {
            mEntitlementRetreiver = [[EntitlementRetriever alloc]init];
            mEntitlementRetreiver.delegate = self;
        }
        
        [mEntitlementRetreiver makeEntitlementsRequest:userInfo];
    }
}


-(void) cancelConnect
{
    [mEntitlementRetreiver cancelRequest];
}


#pragma mark - Entitlement Retriever delegate
-(void) didFailWithErrorMessage:(NSString *)errorMessage
{
    [mDESDialog reset];
    [mDESDialog setErrorText:errorMessage];
}

// after retrieving entitlement url from entitlement service, use endpoint
// to connect to appstream service and retrieve a valid session
-(void) didRetreiveEntitlementUrl:(NSString *)entitlementUrl
{
    [mDESDialog reset];
    [mDESDialog dismiss];
    
    [self showStatus:[NSString stringWithFormat:@"Requesting session from %@ ", entitlementUrl]];
    [self connectXStxClient:entitlementUrl];
    
}

-(void) clearStatus
{
    double delayInSeconds = 2.0;
    dispatch_time_t popTime = dispatch_time(DISPATCH_TIME_NOW, (int64_t)(delayInSeconds * NSEC_PER_SEC));
    dispatch_after(popTime, dispatch_get_main_queue(), ^(void){
        [self showStatus:@""];
    });
}

#pragma mark - AppStreamClientListenerDelegate methods
-(void) clientReady
{
    dispatch_async(dispatch_get_main_queue(), ^{
        if([mDESDialog isDescendantOfView:self.view])
        {
            // use dismiss method so dialog can resignFirstResponder
            [mDESDialog dismiss];
        }
        [self showStatus:@"XStx client ready"];
        [self clearStatus];
    });
    
}

-(void) clientStopped:(NSString *)stopReason
{
    NSLog(@"clientStopped: %@", stopReason);
    [self showError:stopReason];
}

-(void) clientReconnecting:(NSString *) reconnectingMessage withTimeoutInMS:(int) timeoutInMS;
{
    //Make sure this happens on the main thread
    if (![NSThread isMainThread]) {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self clientReconnecting:reconnectingMessage withTimeoutInMS:timeoutInMS];
        });
        return;
    }
    
    NSLog(@"%s %@ %i", __PRETTY_FUNCTION__, reconnectingMessage, timeoutInMS);
    
    //Show the reconnecting view with appropriate message
    [self.view addSubview:_reconnectingView];
    _reconnectingLabel.text = reconnectingMessage;
    [_reconnectingView fadeIn];
    
    [self showStatus:[NSString stringWithFormat:@"XStx attempting reconnect. Timeout: %is", (timeoutInMS/1000)]];
}

-(void) clientReconnected;
{
    if (![NSThread isMainThread]) {
        //Make sure this is running on the main thread
        dispatch_async(dispatch_get_main_queue(), ^{
            [self clientReconnected];
        });
        return;
    }
    [self showStatus:@"XStx client reconnected"];
    
    if ([_reconnectingView isDescendantOfView:self.view])
    {
        [_reconnectingView fadeOutAndRemove];
    }
}

-(void) handleMessage:(NSString *)statusMessage
{
    [self showStatus:statusMessage];
}

-(void) step:(float) fps
{
    [self showFPS:[NSString stringWithFormat:@"%.2f FPS", fps]];
}

@end




