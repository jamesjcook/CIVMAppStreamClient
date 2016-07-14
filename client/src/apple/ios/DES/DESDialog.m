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


#import "DESDialog.h"
#import "constants.h"
#import <QuartzCore/QuartzCore.h>
#import "GradientView.h"   // simple uiview with a gradient in it for drawing backgrounds
#import "InputTextField.h" // input text field subclass to handle spacing
#import "UIView+fade.h"    // UIView category for fading in and out of uiviews
#import "UIButton+gradient.h" // UIButton category for drawing a nice gradient button

// category for private members / functions
@interface DESDialog()
{
    UILabel *appStreamLabel;
    UILabel *mainTitleLabel;
    UILabel *entitlementServiceLabel;
    UILabel *standaloneModeLabel;
    UILabel *appidLabel;
    UILabel *useridLabel;
    UILabel *errorLabel;
    UILabel *spinnerLabel;
    UIView *bgView;
    CGFloat leftmargin;
    CGFloat yoffset;
    
    NSMutableAttributedString *attributedString;
    
    UIButton *submitButton;
    UIButton *toggleStandaloneModeButton;
    
    InputTextField *entitlementServiceTextInput;
    InputTextField *applicationIDTextInput;
    InputTextField *userIDTextInput;
    
    UIView *inputPanel;
    UIView *spinnerPanel;
    
    UIActivityIndicatorView *spinnerView;
    
    BOOL connectionInProgress;
    BOOL standaloneMode;
    
    NSDictionary *formData;
    
    CGFloat midPointX;
    CGFloat midPointY;
    CGFloat width;
    CGFloat height;
    CGFloat topMargin;
    
    BOOL isOnscreenKeyboard;
    BOOL shouldDismiss;
    
}

// send info to our delegate to request entitlmenets
-(void) handleSubmit:(UIButton*) sender;
-(void) toggleStandaloneMode:(UIButton*) sender;
// save form data for next time usage
-(void) cacheFormData:(NSDictionary*) entitlements;
// retreive form data
-(void) populateFromCache;

// enum for labeling fields
typedef enum {
    kDES_SERVER,
    kDES_APPID,
    kDES_USERNAME
} DESInputFieldType;



@end


@implementation DESDialog

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        midPointX = CGRectGetMidX(frame);
        midPointY = CGRectGetMidY(frame);
        width = 320.0f;
        height = 472.0f;
        topMargin = 120.0f;
        self.frame = CGRectMake(0,0,width, height);
        self.center = CGPointMake(midPointX ,midPointY);
        connectionInProgress = false;
        standaloneMode = false;
        formData = nil;
        
        if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone )
        {
            topMargin = 130.0f;
        }
        
        
        [self initGraphics];
        [self populateFromCache];
        [self addObservers];
    }
    return self;
}


-(void) addObservers
{
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillShow:)
                                                 name:UIKeyboardWillShowNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardWillHide:)
                                                 name:UIKeyboardWillHideNotification
                                               object:nil];
    
    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(keyboardDidHide:)
                                                 name:UIKeyboardDidHideNotification
                                               object:nil];
}


// set up the UI for this control
// all of the finicky placement stuff is contained herein

-(void) initGraphics
{
    
    static NSString *boldFontName = @"HelveticaNeue-Bold";
    static NSString *romanFontName = @"HelveticaNeue";
    static NSString *italicFontName = @"Helvetica-Oblique";
    
    CGFloat labelFontSize = 13.0f;
    CGFloat captionFontSize = 12.0f;
    CGFloat verticalSpacing = 75.0f;
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone )
    {
        verticalSpacing = 56.0f;
    }
    
    CGColorRef borderColor = [UIColor colorWithRed:0.729f green:0.733f blue:0.733f alpha:1.0f].CGColor;
    
    self.backgroundColor = [UIColor clearColor];
    bgView = [[UIView alloc]initWithFrame:self.bounds];
    
    bgView.layer.cornerRadius = 8.0f;
    bgView.layer.masksToBounds = YES;
    
    bgView.layer.borderColor = borderColor;
    bgView.layer.borderWidth = 1.0f;
    bgView.backgroundColor = [UIColor whiteColor];
    self.layer.shadowColor = [UIColor blackColor].CGColor;
    self.layer.shadowOpacity = 0.4;
    self.layer.shadowRadius = 25.0f;
    bgView.opaque = NO;
    
    [self addSubview:bgView];
    
    /// PANEL to hold all of the input fields and their associated labels
    // this is so we can fade-in-out the whole group
    inputPanel = [[UIView alloc]initWithFrame:self.bounds];
    inputPanel.backgroundColor = [UIColor clearColor];
    [bgView addSubview:inputPanel];
    
    /// SPINNER PANEL
    // plain panel with a ui activity view spinner
    // for when connections happen
    spinnerLabel =    [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, 0, 140, 20)];
    spinnerLabel.text = @"Connecting...";
    spinnerLabel.backgroundColor  = [UIColor clearColor];
    spinnerLabel.textColor = [UIColor darkGrayColor];
    spinnerLabel.font = [UIFont fontWithName:romanFontName size:captionFontSize];
    spinnerLabel.textAlignment = NSTextAlignmentCenter;
    spinnerPanel = [[UIView alloc]initWithFrame:self.bounds];
    spinnerView = [[UIActivityIndicatorView alloc]initWithActivityIndicatorStyle:UIActivityIndicatorViewStyleGray];
    spinnerView.center = CGPointMake(spinnerPanel.center.x, spinnerPanel.center.y - 20);
    CGPoint spinnerLabelCenter = CGPointMake(spinnerView.center.x, spinnerView.center.y - 30.0f);
    spinnerLabel.center = spinnerLabelCenter;
    [spinnerPanel addSubview:spinnerLabel];
    [spinnerPanel addSubview:spinnerView];
    spinnerPanel.alpha = 0.0f;
    [self addSubview:spinnerPanel];
    
    
    /// this is the gradient background in the header of the panel
    GradientView *headerBackgroundView = [[GradientView alloc]initWithFrame:CGRectMake(0,0,self.bounds.size.width,88.0f)];
    headerBackgroundView.startColor = [UIColor colorWithRed:0.933f green:0.933f blue:0.933f alpha:1.0f];
    headerBackgroundView.endColor = [UIColor colorWithRed:0.863f green:0.882f blue:0.882f alpha:1.0f];
    [bgView addSubview:headerBackgroundView];
    
    // offset from left of control
    leftmargin = 22.0f;
    
    // offset from top
    yoffset = 10.0f;   // will update this as we draw
    
    // Set up all text labels
    // using NSAttributed string for fine-grained typesetting control (particularly letterspacing)
    
    // Amazon AppStream label
    appStreamLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, 18, 140, 20)];
    appStreamLabel.font = [UIFont fontWithName:boldFontName size:labelFontSize];
    
    
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"Amazon AppStream"];
    [attributedString addAttribute:NSKernAttributeName value:@0.02 range:NSMakeRange(0, attributedString.length)];
    [appStreamLabel setAttributedText:attributedString];
    appStreamLabel.backgroundColor = [UIColor clearColor];
    [headerBackgroundView addSubview:appStreamLabel];
    
    yoffset = 32.0f;
    
    // Main Title Label
    mainTitleLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, yoffset, self.bounds.size.width, 40)];
    mainTitleLabel.font = [UIFont fontWithName:romanFontName size:26.0f];
    mainTitleLabel.textColor = [UIColor blackColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"Example Client"];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [mainTitleLabel setAttributedText:attributedString];
    mainTitleLabel.backgroundColor = [UIColor clearColor];
    [headerBackgroundView addSubview:mainTitleLabel];
    
    yoffset = 96.0f;
    
    // Server address label
    entitlementServiceLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, yoffset, self.bounds.size.width, 40)];
    entitlementServiceLabel.font = [UIFont fontWithName:romanFontName size:labelFontSize];
    entitlementServiceLabel.textColor = [UIColor blackColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"Server Address"];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [entitlementServiceLabel setAttributedText:attributedString];
    entitlementServiceLabel.backgroundColor = [UIColor clearColor];
    [inputPanel addSubview:entitlementServiceLabel];
    
    yoffset = 130.0f;
    
    // toggle direct connect
    CGRect toggleStandaloneModeButtonFrame = CGRectMake(leftmargin, yoffset, 28, 28);
    toggleStandaloneModeButton = [UIButton gradientButton:toggleStandaloneModeButtonFrame title:@" "];
    toggleStandaloneModeButton.titleLabel.font  = [UIFont fontWithName:boldFontName size:20];
    [toggleStandaloneModeButton addTarget:self action:@selector(toggleStandaloneMode:) forControlEvents:UIControlEventTouchUpInside];
    [toggleStandaloneModeButton setTitle:@"\u2713" forState:UIControlStateSelected];
    toggleStandaloneModeButton.selected = false;
    [inputPanel addSubview:toggleStandaloneModeButton];
    
    yoffset = 124.0f;
    
    // direct connect
    standaloneModeLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin * 2 + 16, yoffset, self.bounds.size.width, 40)];
    standaloneModeLabel.font = [UIFont fontWithName:romanFontName size:labelFontSize];
    standaloneModeLabel.textColor = [UIColor blackColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"AppStream Stand-Alone Mode"];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [standaloneModeLabel setAttributedText:attributedString];
    standaloneModeLabel.backgroundColor = [UIColor clearColor];
    [inputPanel addSubview:standaloneModeLabel];
    
    yoffset = 204.0f;
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone )
    {
        yoffset -= 14.0f;
    }
    // Application ID label
    appidLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, yoffset, self.bounds.size.width, 40)];
    appidLabel.font = [UIFont fontWithName:romanFontName size:labelFontSize];
    appidLabel.textColor = [UIColor blackColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"Application ID"];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [appidLabel setAttributedText:attributedString];
    appidLabel.backgroundColor = [UIColor clearColor];
    appidLabel.text = @"Application ID";
    [inputPanel addSubview:appidLabel];
    
    
    // User ID
    yoffset += verticalSpacing;
    
    useridLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, yoffset, self.bounds.size.width, 40)];
    useridLabel.font = [UIFont fontWithName:romanFontName size:13.0f];
    useridLabel.textColor = [UIColor blackColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@"User ID"];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [useridLabel setAttributedText:attributedString];
    useridLabel.backgroundColor = [UIColor clearColor];
    [inputPanel addSubview:useridLabel];
    
    //// Now set up text entry fields
    CGFloat inputWidth = self.bounds.size.width - (leftmargin*2);
    CGFloat inputHeight = 28.0f;
    
    
    // ENTITLEMENT SERVICES -- server url
    
    yoffset = 165.0f;
    
    entitlementServiceTextInput = [[InputTextField alloc]initWithFrame:CGRectMake(leftmargin,yoffset,inputWidth, inputHeight )];
    entitlementServiceTextInput.placeholder = PLACEHOLDER_DES_ADDRESS;
    entitlementServiceTextInput.font = [UIFont fontWithName:romanFontName size:captionFontSize];
    entitlementServiceTextInput.tag = kDES_SERVER;
    entitlementServiceTextInput.keyboardType = UIKeyboardTypeURL;
    entitlementServiceTextInput.delegate = self;
    entitlementServiceTextInput.returnKeyType = UIReturnKeyNext;
    
    [inputPanel addSubview:entitlementServiceTextInput];
    
    // APPLICATION ID
    
    yoffset += verticalSpacing;
    
    applicationIDTextInput = [[InputTextField alloc]initWithFrame:CGRectMake(leftmargin,yoffset,inputWidth, inputHeight )];
    applicationIDTextInput.placeholder = PLACEHOLDER_DES_APPID;
    applicationIDTextInput.font = [UIFont fontWithName:romanFontName size:captionFontSize];
    applicationIDTextInput.tag = kDES_APPID;
    applicationIDTextInput.delegate = self;
    applicationIDTextInput.returnKeyType = UIReturnKeyNext;
    [inputPanel addSubview:applicationIDTextInput];
    
    //  USER ID:
    
    yoffset += verticalSpacing;
    
    userIDTextInput = [[InputTextField alloc]initWithFrame:CGRectMake(leftmargin,yoffset,inputWidth, inputHeight )];
    userIDTextInput.placeholder = PLACEHOLDER_DES_USER;
    userIDTextInput.font = [UIFont fontWithName:romanFontName size:captionFontSize];
    userIDTextInput.tag = kDES_USERNAME;
    userIDTextInput.delegate = self;
    userIDTextInput.keyboardType = UIKeyboardTypeEmailAddress;
    userIDTextInput.returnKeyType = UIReturnKeyGo;
    [inputPanel addSubview:userIDTextInput];
    
    
    // on iphone, add a toolbar to ease navigation through text fields
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
    {
        UIToolbar *toolbar = [[UIToolbar alloc] initWithFrame:CGRectMake(0, 0, width, 36)];
        
        UIBarButtonItem *doneButton = [[UIBarButtonItem alloc]
                                       initWithBarButtonSystemItem:UIBarButtonSystemItemDone
                                       target:self action:@selector(resignAll)] ;
        
        UIBarButtonItem *nextButton = [[UIBarButtonItem alloc]
                                       initWithTitle:@"Next" style:UIBarButtonItemStylePlain
                                       target:self action:@selector(nextField)];
        
        UIBarButtonItem *spacer = [[UIBarButtonItem alloc]
                                   initWithBarButtonSystemItem:UIBarButtonSystemItemFlexibleSpace
                                   target:self action:nil];
        
        toolbar.items = [NSArray arrayWithObjects:nextButton, spacer,doneButton,nil];
        
        entitlementServiceTextInput.inputAccessoryView = toolbar;
        applicationIDTextInput.inputAccessoryView = toolbar;
        userIDTextInput.inputAccessoryView = toolbar;
    }
    
    
    // just above the submit button there will be a label to set the error text
    // if there is a login error, or other credentials error etc
    yoffset = userIDTextInput.frame.origin.y + userIDTextInput.frame.size.height-2;
    
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone )
    {
        yoffset -= 12;
        
        
    }
    
    
    errorLabel = [[UILabel alloc]initWithFrame:CGRectMake(leftmargin, yoffset, inputWidth, 52)];
    errorLabel.numberOfLines = 3;
    errorLabel.font = [UIFont fontWithName:italicFontName size:11.0f];
    
    errorLabel.textColor = [UIColor redColor];
    attributedString = [[NSMutableAttributedString alloc] initWithString:@""];
    [attributedString addAttribute:NSKernAttributeName value:@-0.20 range:NSMakeRange(0, attributedString.length)];
    [errorLabel setAttributedText:attributedString];
    errorLabel.backgroundColor = [UIColor clearColor];
    [self addSubview:errorLabel];
    
    // now the submit button
    yoffset = userIDTextInput.frame.origin.y + verticalSpacing;
    CGRect submitButtonFrame = CGRectMake(leftmargin, yoffset, inputWidth, 52);
    submitButton = [UIButton gradientButton:submitButtonFrame title:@"Connect"];
    
    
    // add selector for button action
    [submitButton addTarget:self action:@selector(handleSubmit:) forControlEvents:UIControlEventTouchUpInside];
    
    // add button
    [self addSubview:submitButton];
    
    
    
}


-(void) reset
{
    [self fadeIn];
    connectionInProgress = false;
    [inputPanel fadeIn];
    [spinnerPanel fadeOut];
    [spinnerView stopAnimating];
    [submitButton setTitle:@"Connect" forState:UIControlStateNormal];
    submitButton.enabled = true;
    [submitButton fadeIn];
}

-(void) toggleStandaloneMode:(UIButton*)sender
{
    
    CGFloat heightDiff = 152.0f;
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone )
    {
        heightDiff = 130.0f;
    }
    
    //First cache the current user input data so it can be restored properly
    formData = [[NSDictionary alloc]initWithObjectsAndKeys:
                entitlementServiceTextInput.text ,@"url",
                userIDTextInput.text,@"userid",
                [NSNumber numberWithBool:standaloneMode],@"standalonemode",
                applicationIDTextInput.text, @"appid",nil];
    [self cacheFormData:formData];
    
    
    if( standaloneMode == true)
    {
        standaloneMode = false;
        toggleStandaloneModeButton.selected = false;
        
        [UIView animateWithDuration:0.2 animations:^{
            self.frame = CGRectMake(self.frame.origin.x,self.frame.origin.y,
                                    self.frame.size.width,self.frame.size.height + heightDiff);
            bgView.frame =CGRectMake(bgView.frame.origin.x,bgView.frame.origin.y,
                                     bgView.frame.size.width,bgView.frame.size.height + heightDiff);
            
            entitlementServiceTextInput.placeholder = PLACEHOLDER_DES_ADDRESS;
            entitlementServiceTextInput.returnKeyType = UIReturnKeyNext;
            [self populateFromCache];
            
            appidLabel.alpha = 1.0f;
            applicationIDTextInput.alpha = 1.0f;
            useridLabel.alpha = 1.0f;
            userIDTextInput.alpha = 1.0f;
            
            submitButton.center = CGPointMake(submitButton.center.x,submitButton.center.y + heightDiff);
            errorLabel.center = CGPointMake(submitButton.center.x,errorLabel.center.y + heightDiff);
            
        }];
        
        
    }
    else
    {
        standaloneMode = true;
        toggleStandaloneModeButton.selected = true;
        
        [UIView animateWithDuration:0.2 animations:^{
            
            appidLabel.alpha = 0.0f;
            applicationIDTextInput.alpha = 0.0f;
            useridLabel.alpha = 0.0f;
            userIDTextInput.alpha = 0.0f;
            
            submitButton.center = CGPointMake(submitButton.center.x,submitButton.center.y - heightDiff);
            errorLabel.center = CGPointMake(submitButton.center.x,errorLabel.center.y - heightDiff);
            entitlementServiceTextInput.placeholder = PLACEHOLDER_STANDALONE_ADDRESS;
            entitlementServiceTextInput.returnKeyType = UIReturnKeyGo;
            [self populateFromCache];
            
            self.frame = CGRectMake(self.frame.origin.x,self.frame.origin.y,
                                    self.frame.size.width,self.frame.size.height - heightDiff);
            bgView.frame =CGRectMake(bgView.frame.origin.x,bgView.frame.origin.y,
                                     bgView.frame.size.width,bgView.frame.size.height - heightDiff);
            
        }];
    }
    
    [toggleStandaloneModeButton setNeedsDisplay];
}


-(void) handleSubmit:(UIButton*)sender
{
    
    [self setErrorText:@""];
    
    if( connectionInProgress )
    {
        [self reset];
    }
    else  // start up a connection
    {
        
        // field validation
        NSString *inputString = entitlementServiceTextInput.text;
        inputString = [inputString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        
        NSString *appIDString = applicationIDTextInput.text;
        appIDString = [appIDString stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
        
        NSString *userIDString = userIDTextInput.text;
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
        [inputPanel fadeOut];
        [spinnerView startAnimating];
        [spinnerPanel fadeIn];
        
        //Disable the submit button
        submitButton.enabled = false;
        [submitButton fadeOut];
        
        if(self.delegate && [self.delegate respondsToSelector:@selector(handleConnect:)])
        {
            formData = [[NSDictionary alloc]initWithObjectsAndKeys:
                        inputString ,@"url",
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


-(void) setErrorText:(NSString*) errorMessage
{
    NSLog(@"%@",errorMessage);
    errorLabel.text = errorMessage;
    
}


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
        
        //And the userID
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
                entitlementServiceTextInput.text = urlString;
            }
            else
            {
                entitlementServiceTextInput.text = [hostInfo valueForKey:@"XStxServer"];
            }
            
        }
        else
        {
            if ( desurlString && desurlString.length > 0)
            {
                entitlementServiceTextInput.text = desurlString;
            }
            else
            {
                entitlementServiceTextInput.text = [hostInfo valueForKey:@"DESServer"];
            }
        }
        
        if(appId  && appId.length > 0)
        {
            applicationIDTextInput.text = appId;
        }
        else
        {
            applicationIDTextInput.text = [hostInfo valueForKey:@"AppID"];
        }
        
        
        if( userId && userId.length > 0)
        {
            userIDTextInput.text = userId;
        }
        else
        {
            userIDTextInput.text = [hostInfo valueForKey:@"Username"];
        }
        
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
                applicationIDTextInput.text = appid;
            }
            
            NSString *desserver = [hostInfo valueForKey:@"DESServer"];
            if(! desserver)
            {
                desserver = @"";
            }
            
            if(standaloneMode)
            {
                entitlementServiceTextInput.text = standaloneModeServer ;
            }
            else
            {
                entitlementServiceTextInput.text = desserver;
                
            }
            
            NSString *userid = [hostInfo valueForKey:@"Username"];
            if(userid)
            {
                userIDTextInput.text = userid;
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
            [entitlementServiceTextInput becomeFirstResponder];
        }
    }
    
}



-(void) dismiss
{
    
    
    if ( [  entitlementServiceTextInput isFirstResponder] )
    {
        [entitlementServiceTextInput resignFirstResponder];
    }
    
    if ( [  applicationIDTextInput isFirstResponder] )
    {
        [applicationIDTextInput resignFirstResponder];
    }
    
    if ( [  userIDTextInput isFirstResponder] )
    {
        [applicationIDTextInput resignFirstResponder];
    }
    
    if(! isOnscreenKeyboard)
    {
        [self fadeOutAndRemove];
    }
    else
    {
        shouldDismiss = YES;
    }
}


#pragma mark text field delegate methods


- (BOOL)textFieldShouldBeginEditing:(UITextField *)textField
{
    
    if (UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
    {
        
        CGFloat newY =   height/8 + (height/2 - textField.frame.origin.y);
        if( standaloneMode )
        {
            newY = (height/2 - textField.frame.origin.y);
        }
        
        [UIView animateWithDuration:0.3 animations:^{
            self.center = CGPointMake(midPointX, newY);
        } completion:nil];
        
    }
    
    
    return YES;
}

- (void)textFieldDidBeginEditing:(UITextField *)textField
{
    
}

- (BOOL)textFieldShouldEndEditing:(UITextField *)textField
{
    
    return YES;
}

- (void)textFieldDidEndEditing:(UITextField *)textField
{
    
}

- (BOOL)textField:(UITextField *)textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
    
    return YES;
}

- (BOOL)textFieldShouldClear:(UITextField *)textField
{
    
    return YES;
}

- (BOOL)textFieldShouldReturn:(UITextField *)textField
{
    if (textField.tag == kDES_SERVER)
    {
        if(standaloneMode)
        {
            [self handleSubmit:nil];
        }
        else
        {
            [applicationIDTextInput becomeFirstResponder];
        }
    }
    
    if( textField.tag == kDES_APPID)
    {
        [userIDTextInput becomeFirstResponder];
    }
    
    if( textField.tag == kDES_USERNAME)
    {
        [self handleSubmit:nil];
    }
    
    return YES;
}

-(void) nextField
{
    
    NSArray *textfields = @[entitlementServiceTextInput,applicationIDTextInput,userIDTextInput];
    
    if( standaloneMode == true)
    {
        [textfields[0] becomeFirstResponder];
        return;
    }
    
    
    int nextResponder=0;
    for ( int i=0;i<3;i++)
    {
        if( [textfields[i] isFirstResponder])
        {
            nextResponder = i+1;
            if ( nextResponder == 3)
            {
                nextResponder = 0;
            }
            break;
        }
    }
    [textfields[nextResponder] becomeFirstResponder];
    
    
    
    
}

-(void) resignAll
{
    
    [entitlementServiceTextInput resignFirstResponder];
    [applicationIDTextInput resignFirstResponder];
    [userIDTextInput resignFirstResponder];
    
}


#pragma mark keyboard notifications

-(void) keyboardWillShow:(NSNotification*) notification
{
    
    isOnscreenKeyboard = YES;
    
    if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPad )
    {
        [UIView animateWithDuration:0.3 animations:^{
            self.center = CGPointMake(midPointX, (midPointY + topMargin) - (height/2) );
        } completion:nil];
    }
    
    
}


-(void) keyboardWillHide:(NSNotification*) notification
{
    
    [UIView animateWithDuration:0.3 animations:^{
        self.center = CGPointMake(midPointX ,midPointY);
    }
                     completion:^(BOOL finished)
     {
         isOnscreenKeyboard = NO;
     }];
    
    
}

-(void) keyboardDidHide:(NSNotification*) notification
{
    if( shouldDismiss )
    {
        [self fadeOutAndRemove];
    }
    shouldDismiss = NO;
}

-(void) dealloc
{
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UIKeyboardWillShowNotification object:nil];
    [[NSNotificationCenter defaultCenter] removeObserver:self
                                                    name:UIKeyboardWillHideNotification object:nil];
}

@end



