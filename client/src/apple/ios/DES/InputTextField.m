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


#import "InputTextField.h"

// UITextField subclass for fine-grained typographic control and common settings

@implementation InputTextField

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        
        self.autocapitalizationType = UITextAutocapitalizationTypeNone;
        self.autocorrectionType = UITextAutocorrectionTypeNo;
        self.clearButtonMode = UITextFieldViewModeWhileEditing;
        
        self.layer.borderColor = [UIColor colorWithRed:0.729f green:0.733f blue:0.733f alpha:1.0f].CGColor;
        self.layer.borderWidth = 1.0f;
        self.leftView = [[UIView alloc]initWithFrame:CGRectMake(0,0,6,frame.size.height)];
        self.leftViewMode = UITextFieldViewModeAlways;
        self.leftView.backgroundColor = self.backgroundColor;
        self.superview.clipsToBounds = false;
        self.contentVerticalAlignment = UIControlContentVerticalAlignmentCenter;
    }
    return self;
}

@end
