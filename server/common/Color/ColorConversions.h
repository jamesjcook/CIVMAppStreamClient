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

/**
 * @fn  bool convertToYUV420(const unsigned char* rgbData, int width, int height, int rOffset, int gOffset, int bOffset, int pixelStride, int scanlineStride, uint8_t* yuvPlanes[3]);
 *
 * @brief   Converts provided RGB color space pixel data to YUV420 format
 *          This RGB to YUV conversion is based on the same matrix as
 *          https://github.com/camilleg/unblock/blob/master/main.cpp#L19
 *          and http://www.equasys.de/colorconversion.html YCbCr - RGB.
 *          
 *          which is
 *          
 *          |  0.257 0.504 0.098 |   | R |   | 16  |
 *          | -0.148 0.291 0.439 | X | G | + | 128 |
 *          |  0.439 0.368 0.071 |   | B |   | 128 |
 *          
 *          The corresponding non-floating point operations implemented below are
 *          
 *          |  66  129   25 |   | R |               |  16 |
 *          | -38  -74  112 | X | G | X (1 / 256) + | 128 |
 *          | 112  -94  -18 |   | B |               | 128 |
 *          
 *          Note that the client will have to implement the correct inverse 
 *          transformation to preserve color fidelity across the client and server
 *
 * @param   rgbData             Pointer to the color data in RGB color space
 * @param   width               The width of the image
 * @param   height              The height of the image
 * @param   rOffset             The offset of the R data from the beginning of the pixel
 * @param   gOffset             The offset of the G data from the beginning of the pixel
 * @param   bOffset             The offset of the B data from the beginning of the pixel
 * @param   pixelStride         The pixel stride (omit any components not relevant for ex. A)
 * @param   scanlineStride      The scanline stride
 * @param [in,out]  yuvPlanes   Pointer to the Y, U and V planes that will hold the converted data
 *
 * @return  true if it succeeds, false otherwise
 */

bool convertToYUV420(unsigned char const * const rgbData, 
                     int width, int height,
                     int rOffset, int gOffset, int bOffset, 
                     int pixelStride, int scanlineStride,
                     uint8_t * const yuvPlanes[3]);

/**
 * @fn  bool convertToYUV444(const unsigned char* rgbData, int width, int height, int rOffset, int gOffset, int bOffset, int pixelStride, int scanlineStride, uint8_t* yuvPlanes[3]);
 *
 * @brief   Converts provided RGB color space pixel data to YUV444 format
 *          This RGB to YUV conversion is based on the same matrix as
 *          https://github.com/camilleg/unblock/blob/master/main.cpp#L19
 *          and http://www.equasys.de/colorconversion.html YCbCr - RGB.
 *          
 *          which is
 *          
 *          |  0.257 0.504 0.098 |   | R |   | 16  |
 *          | -0.148 0.291 0.439 | X | G | + | 128 |
 *          |  0.439 0.368 0.071 |   | B |   | 128 |
 *          
 *          The corresponding non-floating point operations implemented below are
 *          
 *          |  66  129   25 |   | R |               |  16 |
 *          | -38  -74  112 | X | G | X (1 / 256) + | 128 |
 *          | 112  -94  -18 |   | B |               | 128 |
 *          
 *          Note that the client will have to implement the correct inverse 
 *          transformation to preserve color fidelity across the client and server
 *
 * @param   rgbData             Pointer to the color data in RGB color space
 * @param   width               The width of the image
 * @param   height              The height of the image
 * @param   rOffset             The offset of the R data from the beginning of the pixel
 * @param   gOffset             The offset of the G data from the beginning of the pixel
 * @param   bOffset             The offset of the B data from the beginning of the pixel
 * @param   pixelStride         The pixel stride (omit any components not relevant for ex. A)
 * @param   scanlineStride      The scanline stride
 * @param [in,out]  yuvPlanes   Pointer to the Y, U and V planes that will hold the converted data
 *
 * @return  true if it succeeds, false otherwise
 */
bool convertToYUV444(unsigned char const * const rgbData,
                     int width, int height,
                     int rOffset, int gOffset, int bOffset,
                     int pixelStride, int scanlineStride,
                     uint8_t * const yuvPlanes[3]);
