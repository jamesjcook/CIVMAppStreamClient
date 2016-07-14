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


#import "UIButton+gradient.h"
#import "GradientView.h"

@implementation UIButton (gradient)

/* factory method to create a new gradient button */

+(UIButton*) gradientButton:(CGRect ) frame
                      title:(NSString*) title
{
    return [UIButton gradientButton:frame title:title borderColor:nil startColor:nil endColor:nil];
}



+(UIButton*) gradientButton:(CGRect ) frame
                      title:(NSString*) title
                borderColor:(UIColor*) borderColor
                 startColor:(UIColor*) startColor
                   endColor:(UIColor*) endColor;
{
    

    
    UIColor *gradientStartColor;
    UIColor *gradientEndColor;
    
    if(borderColor == nil)
    {
        borderColor = [UIColor colorWithRed:0.729f green:0.733f blue:0.733f alpha:1.0f];
    }
    
    if(startColor == nil)
    {
        gradientStartColor = [UIColor colorWithRed:1.0f green:1.0f blue:1.0f alpha:1.0f];
    }
    
    if(endColor == nil)
    {
        gradientEndColor = [UIColor colorWithRed:0.823f green:0.822f blue:0.822f alpha:1.0f];
    }
    
    // create a button of type custom and set up its title and border
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = frame;
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    button.layer.borderColor = borderColor.CGColor;
    button.layer.cornerRadius = 5.0f;
    button.layer.borderWidth = 1.0f;
    button.layer.masksToBounds = YES;
    
    /// now draw two gradients into two images which we will set for the buttons background images for control state
    
    // light to dark, top to bottom
    GradientView *buttonGradientNormal = [[GradientView alloc]initWithFrame:button.frame];
    buttonGradientNormal.startColor = gradientStartColor;
    buttonGradientNormal.endColor = gradientEndColor;
    UIGraphicsBeginImageContextWithOptions(button.frame.size, YES, 0.0);
    [buttonGradientNormal.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *buttonBackgroundImageNormal = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    // dark to light, top to bottom (the reverse)
    GradientView *buttonGradientHighlighted = [[GradientView alloc]initWithFrame:button.frame];
    buttonGradientHighlighted.endColor = gradientStartColor;
    buttonGradientHighlighted.startColor = gradientEndColor;
    UIGraphicsBeginImageContextWithOptions(button.frame.size, YES, 0.0);
    [buttonGradientHighlighted.layer renderInContext:UIGraphicsGetCurrentContext()];
    UIImage *buttonBackgroundImageHighlighted = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
    
    
    // set the in-memory images to the button states
    [button setBackgroundImage:buttonBackgroundImageNormal forState:UIControlStateNormal];
    [button setBackgroundImage:buttonBackgroundImageHighlighted forState:UIControlStateHighlighted];
    [button setBackgroundImage:buttonBackgroundImageHighlighted forState:UIControlStateSelected];
    
    
    return button;
}


+(UIButton*) borderlessStyleButton:(CGRect) frame
                           title:(NSString*) title
{
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = frame;
    button.backgroundColor = [UIColor clearColor];
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    button.titleLabel.font = [UIFont fontWithName:@"HelveticaNeue-Light" size:24.0f];
    button.layer.cornerRadius = 8.0f;
    button.layer.masksToBounds = YES;
    
    return button;
    
}


+(UIButton*) keyboardStyleButton:(CGRect) frame
                           title:(NSString*) title
{
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = frame;
    [button setTitle:title forState:UIControlStateNormal];
    [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    button.titleLabel.font = [UIFont fontWithName:@"HelveticaNeue-Light" size:27.0f];
    button.layer.borderColor = [UIColor darkGrayColor].CGColor;
    button.layer.cornerRadius = 6.0f;
    
    if ( UI_USER_INTERFACE_IDIOM() == UIUserInterfaceIdiomPhone)
    {
        button.layer.cornerRadius = 3.0f;
    }
    
    button.layer.masksToBounds = YES;
    
    UIView *shadowview = [[UIView alloc]initWithFrame:button.frame];
    shadowview.layer.cornerRadius = button.layer.cornerRadius;
    shadowview.backgroundColor =  [UIColor colorWithRed:0.545f green:0.545f blue:0.549f alpha:1.0f];
    
    
    // layer two uiviews to create shadow effect
    CGRect overlayFrame = CGRectOffset(shadowview.bounds, 0.0f, -1.0f);
    UIView *bgview = [[UIView alloc]initWithFrame:overlayFrame];
    bgview.layer.cornerRadius = button.layer.cornerRadius;
    bgview.layer.masksToBounds = YES;
    bgview.backgroundColor = [UIColor whiteColor];
    [shadowview addSubview:bgview];
    
    // render images and assign to states
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateNormal];
    UIGraphicsEndImageContext();
    
    bgview.backgroundColor  = [UIColor colorWithRed:0.816f green:0.827f blue:0.843f alpha:1.0f];
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateHighlighted];
    UIGraphicsEndImageContext();
    
    bgview.backgroundColor = [UIColor lightGrayColor];
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateSelected];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateHighlighted];
    UIGraphicsEndImageContext();
    
    return button;
}

/**
 *
 */

+(UIButton*) keyboardStyleImageButton:(CGRect) frame
                                image:(NSString*) imageName
                     highlightedImage:(NSString*) highlightImageName
                        selectedImage:(NSString*) selectedImageName
{
    
    UIButton *button = [UIButton buttonWithType:UIButtonTypeCustom];
    button.frame = frame;
    [button setTitle:@"" forState:UIControlStateNormal];
    [button setTitleColor:[UIColor blackColor] forState:UIControlStateNormal];
    button.titleLabel.font = [UIFont fontWithName:@"HelveticaNeue-Light" size:24.0f];
    button.layer.borderColor = [UIColor darkGrayColor].CGColor;
    button.layer.cornerRadius = 6.0f;
    button.layer.shadowColor = [UIColor darkGrayColor].CGColor;
    button.layer.masksToBounds = YES;
    
    UIView *shadowview = [[UIView alloc]initWithFrame:button.frame];
    shadowview.layer.cornerRadius = button.layer.cornerRadius;
    shadowview.backgroundColor =  [UIColor colorWithRed:0.545f green:0.545f blue:0.549f alpha:1.0f];
    
    // layer two uiviews to create shadow effect
    CGRect overlayFrame = CGRectOffset(shadowview.bounds, 0.0f, -1.0f);
    UIView *bgview = [[UIView alloc]initWithFrame:overlayFrame];
    bgview.layer.cornerRadius = button.layer.cornerRadius;
    bgview.layer.masksToBounds = YES;
    bgview.backgroundColor = [UIColor whiteColor];
    [shadowview addSubview:bgview];
    
    // render images and assign to states
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateNormal];
    UIGraphicsEndImageContext();
    
    bgview.backgroundColor  = [UIColor colorWithRed:0.816f green:0.827f blue:0.843f alpha:1.0f];
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateHighlighted];
    UIGraphicsEndImageContext();
    
    bgview.backgroundColor = [UIColor lightGrayColor];
    UIGraphicsBeginImageContextWithOptions(frame.size, YES, 0.0);
    [shadowview.layer renderInContext:UIGraphicsGetCurrentContext()];
    [button setBackgroundImage:UIGraphicsGetImageFromCurrentImageContext() forState:UIControlStateSelected];
    UIGraphicsEndImageContext();
    
  
    
    
    UIImage *normalImage=[UIImage imageNamed:imageName];
    
    UIImage *highlightImage = [UIImage imageNamed:highlightImageName];
    
    UIImage *selectImage = [UIImage imageNamed:selectedImageName];
    
    [[button imageView] setContentMode: UIViewContentModeScaleAspectFit];

    
    if(imageName)
    {
        [button setImage:normalImage forState:UIControlStateNormal];
    }
    
    if(selectedImageName)
    {
        [button setImage:selectImage forState:UIControlStateSelected];
    }
    
    if(highlightImage)
    {
        [button setImage:highlightImage forState:UIControlStateHighlighted];
    }
    
    return button;
    
}


@end
