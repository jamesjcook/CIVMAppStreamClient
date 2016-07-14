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


/////////////////////////////////////////////
// Draw a simple linear gradient top to bottom
// from start color to end color
/////////////////////////////////////////////

#import "GradientView.h"

@implementation GradientView

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
            }
    return self;
}


- (void)drawRect:(CGRect)rect
{
    // pointer to graphics context
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    // set default black and white
    CGFloat colors [] = {
        0.0, 0.0, 0.0, 1.0, // start
        1.0, 1.0, 1.0, 1.0  // end
    };
    
    // if we do not get passed in a start or end color, set our member vars to white / black
    
    if(! _startColor){
        _startColor = [UIColor whiteColor];
    }
    if(! _endColor){
        _endColor = [UIColor blackColor];
    }
    
    
    // gather colors' components
    CGFloat startRed,startGreen,startBlue,startAlpha;
    CGFloat endRed,endGreen,endBlue,endAlpha;
    
    BOOL gotStartColor = [_startColor getRed:&startRed green:&startGreen blue:&startBlue alpha:&startAlpha];
    BOOL gotEndColor = [_endColor getRed:&endRed green:&endGreen blue:&endBlue alpha:&endAlpha];
    
    
    // if the colors were passed in, then use them to set up the colors array
    if( gotStartColor && gotEndColor )
    {
        
        ////// this is the start color, R, G, B, A
        colors [0] = startRed;
        colors [1] = startGreen;
        colors [2] = startBlue;
        colors [3] = startAlpha;
        
        ////// this is the end color, R, G, B, A
        colors [4] = endRed;
        colors [5] = endGreen;
        colors [6] = endBlue;
        colors [7] = endAlpha;
       
    }
    
    // set up to draw the gradient
    CGColorSpaceRef colorspace = CGColorSpaceCreateDeviceRGB();
    CGGradientRef gradient = CGGradientCreateWithColorComponents(colorspace, colors, NULL, 2);
    
    
    /// set locations of gradient points -- top and bottom midpoints
    CGPoint startPoint = CGPointMake(CGRectGetMidX(rect), CGRectGetMinY(rect));
    CGPoint endPoint = CGPointMake(CGRectGetMidX(rect), CGRectGetMaxY(rect));
    
    // now draw the gradient
    CGContextDrawLinearGradient(context, gradient, startPoint, endPoint, 0);
    
    // clean up
    CGColorSpaceRelease(colorspace);
    CGGradientRelease(gradient);
    
    gradient = NULL;
    colorspace = NULL;
    
   
}



@end
