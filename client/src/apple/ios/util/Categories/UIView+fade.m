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


#import "UIView+fade.h"

@implementation UIView (fade)


-(void) hide
{
    [self fadeOutAndRemove];
}

-(void) show
{
    self.hidden = NO;
    [self fadeIn];
}

-(void) fadeIn
{
    [self.superview bringSubviewToFront:self];
    [UIView animateWithDuration:0.2f animations:^{
        self.alpha = 1.0f;
    }];
    
}
-(void) fadeOut
{
    [UIView animateWithDuration:0.2f animations:^{
        self.alpha = 0.0f;
    }];
}

-(void) fadeOutIn
{
    [UIView animateWithDuration:0.2f animations:^{
        self.alpha = 0.0f;
    } completion:^(BOOL completed){
        if ( completed)
        {
            [self fadeIn];
        }
    }];
}

-(void) fadeOutAndRemove
{
    [UIView animateWithDuration:0.15f animations:^{
        self.alpha = 0.0f;
    } completion:^(BOOL finished) {
        if (finished) {
            [self removeFromSuperview];
        }
    }];
}


-(void) addSubviewWithFade:(UIView *)view;
{
    view.alpha = 0.0f;
    [self addSubview:view];
    [view fadeIn];
    
}


@end
