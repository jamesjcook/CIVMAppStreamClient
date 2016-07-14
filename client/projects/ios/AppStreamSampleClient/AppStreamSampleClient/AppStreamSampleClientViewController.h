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


#import <UIKit/UIKit.h>
#import <GLKit/GLKit.h>

// XSTX classes
#include "XStx/client/XStxClientAPI.h"

// Developer entitlement service includes
#import "DESDialog.h"  // dialog box
#import "EntitlementRetriever.h"  // module to request entitlements

// AppStream client singleton
#import "AppStreamClient.h"

// Input text field delegate (for capturing backspace on keyboard entry)
#import "AppStreamInputTextField.h"

@interface AppStreamSampleClientViewController : GLKViewController
    <DESDialogDelegate, EntitlementRetrieverDelegate,AppStreamClientListenerDelegate, AppStreamInputTextFieldDelegate>

@property (weak, nonatomic) IBOutlet UILabel *statusLabel;
@property (weak, nonatomic) IBOutlet UILabel *fpsLabel;
@property (weak, nonatomic) IBOutlet UIButton *keyboardButton;
@property (weak, nonatomic) IBOutlet UIButton *stopButton;
@property (weak, nonatomic) IBOutlet AppStreamInputTextField *inputTextField;
@property (strong, nonatomic) IBOutlet UIImageView *backgroundImageView;
@property (strong, nonatomic) IBOutlet UIView *reconnectingView;
@property (weak, nonatomic) IBOutlet UILabel *reconnectingLabel;
- (IBAction)toggleKeyboard:(id)sender;
- (IBAction)pressedStop:(id)sender;


@end


