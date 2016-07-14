/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#pragma once

// Convert from RGB(A) interleaved to YUV420 planar
bool convertToYUV420(uint8_t const * const rgbData, 
					 int width, int height,
					 int rOffset, int gOffset, int bOffset, 
					 int pixelStride, int scanlineStride,
					 uint8_t* const yuvPlanes[3]);

// Convert from RGB(A) interleaved to YUV444 planar
bool convertToYUV444(uint8_t const * const rgbData, 
					 int width, int height,
					 int rOffset, int gOffset, int bOffset, 
					 int pixelStride, int scanlineStride,
					 uint8_t* const yuvPlanes[3]);