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


#import <Cocoa/Cocoa.h>

@interface DESDialogWindowController : NSWindowController

@property (weak) IBOutlet NSTextField *entitlementServiceTextField;
@property (weak) IBOutlet NSTextField *applicationIDTextField;
@property (weak) IBOutlet NSTextField *userIDTextField;
@property (weak) IBOutlet NSButton *toggleStandaloneModeButton;
@property (weak) IBOutlet NSButton *connectButton;
@property (weak) IBOutlet NSTextField *entitlementServerLabel;
@property (weak) IBOutlet NSTextField *applicationIDLabel;
@property (weak) IBOutlet NSTextField *userIDLabel;
- (IBAction)handleSubmit:(id)sender;
- (IBAction)toggleStandaloneMode:(NSButton *)sender;
@property (weak) IBOutlet NSView *whiteBGView;
@property (weak) IBOutlet NSView *desInputHolderView;
@property (weak) IBOutlet NSView *spinnerPanel;
@property (weak) IBOutlet NSProgressIndicator *spinnerView;


@property (nonatomic, weak) NSString *entitlementServerURL;
@property (nonatomic, weak) NSString *applicationID;
@property (nonatomic, weak) NSString *userID;
@property (weak) IBOutlet NSTextField *errorLabel;
@property (nonatomic, weak) id delegate;

// public methods
-(void) setErrorText:(NSString*) errorMessage;
-(void) reset;
-(void) dismiss;

@end


// protocol for communicating with
// app to handle connections and disconnection

@protocol DESDialogDelegate <NSObject>
@optional
//
-(void) handleConnect:(NSDictionary*) userInfo;
-(void) cancelConnect;

@end
