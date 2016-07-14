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


#import "constants.h"
#import "DESDialogWindowController.h"

#import <QuartzCore/QuartzCore.h>

@interface DESDialogWindowController ()
{
    BOOL connectionInProgress;
    BOOL standaloneMode;
}

-(void) cacheFormData:(NSDictionary*) entitlements;
// retreive form data
-(void) populateFromCache;


@end

@implementation DESDialogWindowController

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
    
    [self populateFromCache];
    
    [self.whiteBGView setWantsLayer:true];
    self.whiteBGView.layer.backgroundColor = [NSColor whiteColor].CGColor;
    
    self.spinnerPanel.alphaValue = 0;
}

-(void) setErrorText:(NSString*) errorMessage;
{
    NSLog(@"ErrorText: %@", errorMessage);
    [_errorLabel setStringValue:errorMessage];
}

-(void) reset
{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    connectionInProgress = false;
    self.desInputHolderView.alphaValue = 1.0;
    self.spinnerPanel.alphaValue = 0;
    [self.spinnerView stopAnimation:self];
    [self.connectButton setTitle:@"Connect"];
    [self.connectButton setEnabled:true];
    self.connectButton.alphaValue = 1.0;
}

-(void) dismiss;
{
    NSLog(@"%s", __PRETTY_FUNCTION__);
    [self.window close];
}

- (IBAction)handleSubmit:(id)sender {
    NSLog(@"%s", __PRETTY_FUNCTION__);
    [self setErrorText:@""];
    
    if( connectionInProgress )
    {
        [self reset];
    }
    else  // start up a connection
    {
        // field validation
        NSString *inputString = _entitlementServiceTextField.stringValue;
        inputString = [inputString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        
        NSString *appIDString = _applicationIDTextField.stringValue;
        appIDString = [appIDString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

        NSString *userIDString = _userIDTextField.stringValue;
        userIDString = [userIDString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];

        // Make sure there is something in the server address field
        if(inputString.length == 0)
        {
            [self setErrorText:ERROR_MISSING_SERVER_ADDRESS];
            return;
        }
        
        if(standaloneMode)
        {
            //Only validate against empty string (done above)
            // so nothing else to validate here
        }
        else
        {
            // check for a valid http:// based url
            NSString *testHttpURLString = nil;
            if (! [inputString hasPrefix:@"http"])
            {
                testHttpURLString = [NSString stringWithFormat:@"http://%@",inputString];
            }
            
            //Simple validation on the URL entered. Ensures it is http(s):// followed by a domain with a tld or a valid looking IP address
            NSString *urlRegEx = @"^(http[s]?://)?([a-zA-Z0-9]([a-zA-Z0-9\\-]{0,61}[a-zA-Z0-9])?\\.)+[a-zA-Z]{2,6}(:[0-9]+)?(/.*)?$";
            NSString *ipRegEx = @"^(http[s]?://)?\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}[.]\\d{1,3}(:[0-9]+)?(/.*)?$";
            NSPredicate *urlTest = [NSPredicate predicateWithFormat:@"SELF MATCHES %@", urlRegEx];
            BOOL passesURLTest = [urlTest evaluateWithObject:testHttpURLString];
            NSPredicate *ipTest = [NSPredicate predicateWithFormat:@"SELF MATCHES %@", ipRegEx];
            BOOL passesIPTest = [ipTest evaluateWithObject:testHttpURLString];
            
            if (!(passesURLTest || passesIPTest)) {
                //URL does not appear to be valid
                [self setErrorText:ERROR_INVALID_SERVER_ADDRESS];
                return;
            }
            
            if(appIDString.length == 0)
            {
                [self setErrorText:ERROR_MISSING_APP_ID];
                return;
            }
            
            if(userIDString.length == 0)
            {
                [self setErrorText:ERROR_MISSING_USER_ID];
                return;
            }
            
        }
        
        
        
        connectionInProgress = true;
        
        self.desInputHolderView.alphaValue = 0.0;
        self.spinnerPanel.alphaValue = 1.0;
        [self.spinnerView startAnimation:self];
        [self.connectButton setEnabled:false];
        self.connectButton.alphaValue = 0.0;
        
        [self.window makeFirstResponder:nil];
        
        
        if(self.delegate && [self.delegate respondsToSelector:@selector(handleConnect:)])
        {
            NSDictionary *formData = [[NSDictionary alloc]initWithObjectsAndKeys:
                        _entitlementServiceTextField.stringValue ,@"url",
                        userIDString,@"userid",
                        [NSNumber numberWithBool:standaloneMode],@"standalonemode",
                        appIDString, @"appid",nil];
            
            [self.delegate handleConnect:formData];
            [self cacheFormData:formData];
            
        }
        else
        {
            NSLog(@"Error - no delegate set for DESDialog. ");
        }
    } // conn in progress
}

- (IBAction)toggleStandaloneMode:(NSButton *)sender {
    NSLog(@"%s - %li", __PRETTY_FUNCTION__, sender.state);
    
    NSDictionary *formData = [[NSDictionary alloc]initWithObjectsAndKeys:
                              _entitlementServiceTextField.stringValue ,@"url",
                              _userIDTextField.stringValue,@"userid",
                              [NSNumber numberWithBool:standaloneMode],@"standalonemode",
                              _applicationIDTextField.stringValue, @"appid",nil];
    [self cacheFormData:formData];

    
    standaloneMode = (sender.state == NSOnState);
    
    CGFloat heightDiff = 152.0f;

    //Re-populate the defaults from the cache
    [self populateFromCache];
    
    //New frame size
    CGRect newFrame = CGRectZero;
    //Target alpha for the text input stuff
    CGFloat targetAlpha = 1.0;
    
    if (standaloneMode) {
        //  less fields -> make dialog smaller
        newFrame = CGRectMake(self.window.frame.origin.x, self.window.frame.origin.y + heightDiff, self.window.frame.size.width, self.window.frame.size.height - heightDiff);

        targetAlpha = 0.0;

        [[_entitlementServiceTextField cell] setPlaceholderString:PLACEHOLDER_STANDALONE_ADDRESS];
    } else
    {
        // make dialog bigger to accommodate extra fields
        newFrame = CGRectMake(self.window.frame.origin.x, self.window.frame.origin.y - heightDiff, self.window.frame.size.width, self.window.frame.size.height + heightDiff);
        
        targetAlpha = 1.0;
        
        [[_entitlementServiceTextField cell] setPlaceholderString: PLACEHOLDER_DES_ADDRESS];
    }
    
    //Setup an animation transaction for the UI changes
    [CATransaction begin];
    [CATransaction setAnimationDuration:0.5];
    [CATransaction setAnimationTimingFunction:[CAMediaTimingFunction functionWithName:kCAMediaTimingFunctionEaseOut]];

    _applicationIDTextField.animator.alphaValue = targetAlpha;
    _applicationIDLabel.animator.alphaValue = targetAlpha;
    _userIDTextField.animator.alphaValue = targetAlpha;
    _userIDLabel.animator.alphaValue = targetAlpha;
    
    [[self.window animator] setFrame:newFrame display:true];

    //Commit the animation
    [CATransaction commit];
}


#pragma mark - Form Data Caching
-(void) cacheFormData:(NSDictionary*) entitlements
{
    
    NSDictionary *cachedCredentials = nil;
    cachedCredentials = (NSDictionary*)[[NSUserDefaults standardUserDefaults]
                                        objectForKey:@"cachedEntitlements"];
    
    
    NSMutableDictionary *credentialsToCache = nil;
    if( cachedCredentials != nil)
    {
        credentialsToCache = [NSMutableDictionary dictionaryWithDictionary:cachedCredentials];
        
    }else {
        credentialsToCache = [NSMutableDictionary dictionaryWithDictionary:entitlements];
    }
    
    if(standaloneMode)
    {
        
        NSString *urlstring = [ entitlements valueForKey:@"url"];
        if(!urlstring)
        {
            urlstring = @"";
        }
        [credentialsToCache setObject:urlstring forKey:@"url" ];
        
        
    }
    else
    {
        // there is only one 'url' input field , so save desurl string from
        // url value in the entitlements dictionary, which contains the form info
        
        NSString *desurlstring = [ entitlements valueForKey:@"url"];
        if( ! desurlstring)
        {
            desurlstring = @"";
        }
        [credentialsToCache setObject:desurlstring forKey:@"desurl" ];
        
        //Also cache the appID
        NSString *appidstring = [entitlements objectForKey:@"appid"];
        if (!appidstring) {
            appidstring = @"";
        }
        [credentialsToCache setObject:appidstring forKey:@"appid"];
        
        NSString *useridstring = [entitlements objectForKey:@"userid"];
        if (!useridstring) {
            useridstring = @"";
        }
        [credentialsToCache setObject:useridstring forKey:@"userid"];
    }
    
    
    [[NSUserDefaults standardUserDefaults] setObject:credentialsToCache forKey:@"cachedEntitlements"];
    [[NSUserDefaults standardUserDefaults] synchronize];
    
}

-(void) populateFromCache
{
    
    NSDictionary *cachedCredentials = (NSDictionary*)[[NSUserDefaults standardUserDefaults]objectForKey:@"cachedEntitlements"];
    
    
    NSString *filepath = [[NSBundle mainBundle]
                          pathForResource:@"HostInfo" ofType:@"plist"];
    NSDictionary *hostInfo = [NSDictionary
                              dictionaryWithContentsOfFile:filepath];
    
    if( cachedCredentials != nil){
        // retreive values from userInfo dict (passed in
        NSString *urlString = [cachedCredentials valueForKey:@"url"];
        NSString *desurlString = [cachedCredentials valueForKey:@"desurl"];
        NSString *appId = [cachedCredentials valueForKey:@"appid"];
        NSString *userId = [cachedCredentials valueForKey:@"userid"];
        
        
        if(standaloneMode)
        {
            if( urlString && urlString.length > 0)
            {
                _entitlementServiceTextField.stringValue = urlString;
            }
            else
            {
                _entitlementServiceTextField.stringValue = [hostInfo valueForKey:@"XStxServer"];
            }
            [_entitlementServiceTextField.cell setPlaceholderString:PLACEHOLDER_STANDALONE_ADDRESS];
        }
        else
        {
            if ( desurlString && desurlString.length > 0)
            {
                _entitlementServiceTextField.stringValue = desurlString;
            }
            else
            {
                _entitlementServiceTextField.stringValue = [hostInfo valueForKey:@"DESServer"];
            }
            [_entitlementServiceTextField.cell setPlaceholderString:PLACEHOLDER_DES_ADDRESS];
        }
        
        if(appId  && appId.length > 0)
        {
            _applicationIDTextField.stringValue = appId;
        }
        else
        {
            _applicationIDTextField.stringValue = [hostInfo valueForKey:@"AppID"];
        }
        [_applicationIDTextField.cell setPlaceholderString:PLACEHOLDER_DES_APPID];
        
        
        if( userId && userId.length > 0)
        {
            _userIDTextField.stringValue = userId;
        }
        else
        {
            _userIDTextField.stringValue = [hostInfo valueForKey:@"Username"];
        }
        [_userIDTextField.cell setPlaceholderString:PLACEHOLDER_DES_USER];
        
    }
    else // no cached data, look up default info in HostInfo.plist
    {
        
        if(hostInfo)
        {
            
            NSString *standaloneModeServer = [hostInfo valueForKey:@"XStxServer"];
            if(! standaloneModeServer)
            {
                standaloneModeServer = @"";
            }
            
            
            NSString *appid = [hostInfo valueForKey:@"AppID"];
            if (appid)
            {
                _applicationIDTextField.stringValue = appid;
            }
            
            NSString *desserver = [hostInfo valueForKey:@"DESServer"];
            if(! desserver)
            {
                desserver = @"";
            }
            
            if(standaloneMode)
            {
                _entitlementServiceTextField.stringValue = standaloneModeServer ;
            }
            else
            {
                _entitlementServiceTextField.stringValue = desserver;
                
            }
            
            NSString *userid = [hostInfo valueForKey:@"Username"];
            if(userid)
            {
                _userIDTextField.stringValue = userid;
            }
            
            NSDictionary *dict = [[NSDictionary alloc]initWithObjectsAndKeys:
                                  standaloneModeServer,@"url",
                                  desserver,@"desurl",
                                  userid ,@"userid",
                                  [NSNumber numberWithBool:standaloneMode],@"standalonemode",
                                  appid, @"appid",nil];
            
            [[NSUserDefaults standardUserDefaults] setObject:dict forKey:@"cachedEntitlements"];
            [[NSUserDefaults standardUserDefaults] synchronize];
            
            
        }
        else
        {
            [_entitlementServiceTextField becomeFirstResponder];
        }
    }
    
}
@end
