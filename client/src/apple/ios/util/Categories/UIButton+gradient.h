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

@interface UIButton (gradient)

+(UIButton*) gradientButton:(CGRect ) frame
                      title:(NSString*) title;


+(UIButton*) gradientButton:(CGRect ) frame
                      title:(NSString*) title
                borderColor:(UIColor*) borderColor
                 startColor:(UIColor*) startColor
                   endColor:(UIColor*) endColor;

+(UIButton*) keyboardStyleButton:(CGRect) frame
                           title:(NSString*) title;

+(UIButton*) keyboardStyleImageButton:(CGRect) frame
                                image:(NSString*) imageName
                     highlightedImage:(NSString*) highlightImageName
                        selectedImage:(NSString*) selectedImageName;

+(UIButton*) borderlessStyleButton:(CGRect) frame
                           title:(NSString*) title;

@end
