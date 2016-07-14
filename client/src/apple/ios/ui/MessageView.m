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


#import "MessageView.h"
#import <QuartzCore/QuartzCore.h>

@interface MessageView()

-(void) setup;

@end

@implementation MessageView
{
    
    UILabel *label;
    NSString *text;
    NSTimer *displayTimer;
    
};

- (id) initWithFrame:(CGRect ) frame andMessage:(NSString*)message
{
    self = [super initWithFrame:frame];
    if (self) {
        text = message;
        [self setup];
        self.userInteractionEnabled = NO;
        self.clipsToBounds = YES;
    }
    return self;
    
}

-(void) setup
{
    [self setUserInteractionEnabled:NO];
    self.backgroundColor = [UIColor colorWithRed:0.3 green:0.3 blue:0.3 alpha:0.6];
    self.layer.cornerRadius = 4.0f;
    self.layer.borderColor = [UIColor blackColor].CGColor;
    self.layer.borderWidth = 1.0f;
    self.layer.masksToBounds = YES;
    self.layer.shadowColor = [UIColor blackColor].CGColor;
    self.layer.shadowOpacity = 0.7f;
    self.layer.shadowRadius = 12.0f;
    label = [[UILabel alloc]initWithFrame:self.bounds];
    label.textColor = [UIColor yellowColor];
    label.textAlignment = NSTextAlignmentCenter;
    label.text = text;
    [self addSubview:label];
    self.opaque = NO;
}

-(void) setMessage:(NSString *) message
{
    label.alpha = 1.0;
    text = message;
    label.text = text;
}

@end
