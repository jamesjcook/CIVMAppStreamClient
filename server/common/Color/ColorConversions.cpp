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

#include <stdint.h>

#include "ColorConversions.h"

bool convertToYUV420(unsigned char const * const rgbData, 
                     int width, int height,
                     int rOffset, int gOffset, int bOffset, 
                     int pixelStride, int scanlineStride,
                     uint8_t * const yuvPlanes[3])
{
    // This RGB to YUV conversion is based on the same matrix as
    // https://github.com/camilleg/unblock/blob/master/main.cpp#L19
    // and http://www.equasys.de/colorconversion.html YCbCr - RGB.
    // 
    // which is
    // 
    // |  0.257 0.504 0.098 |   | R |   | 16  |
    // | -0.148 0.291 0.439 | X | G | + | 128 |
    // |  0.439 0.368 0.071 |   | B |   | 128 |
    // 
    // The corresponding non-floating point operations implemented below are
    // 
    // |  66  129   25 |   | R |               |  16 |
    // | -38  -74  112 | X | G | X (1 / 256) + | 128 |
    // | 112  -94  -18 |   | B |               | 128 |
    // 
    // Note that the client will have to implement the correct inverse 
    // transformation to preserve color fidelity across the client and server

    try 
    {
        unsigned char const * bytesRowStart = rgbData;
        int const widthHalf = width >> 1;
        int const pixelStr2 = pixelStride << 1;

        for (int h = 0; h < height; h++)
        {
            uint8_t * yAddr = yuvPlanes[0] + h * width;
            uint8_t * uAddr = yuvPlanes[1] + ( h >> 1 ) * widthHalf;
            uint8_t * vAddr = yuvPlanes[2] + ( h >> 1 ) * widthHalf;
            unsigned char const * rAddr = bytesRowStart + rOffset;
            unsigned char const * gAddr = bytesRowStart + gOffset;
            unsigned char const * bAddr = bytesRowStart + bOffset;
            if (h % 2 ==0) {
                for (int w = 0; w < widthHalf; w++ )
                {
                    unsigned char const rValue1 = *rAddr;
                    unsigned char const gValue1 = *gAddr;
                    unsigned char const bValue1 = *bAddr;

                    unsigned char const rValue2 = *(rAddr + pixelStride);
                    unsigned char const gValue2 = *(gAddr + pixelStride);
                    unsigned char const bValue2 = *(bAddr + pixelStride);

                    *yAddr++ = (( 66*rValue1 + 129*gValue1 + 25*bValue1) >> 8) + 16;
                    *yAddr++ = (( 66*rValue2 + 129*gValue2 + 25*bValue2) >> 8) + 16;
                    *uAddr++ = ((-19*((int)(rValue1)+(int)(rValue2)) - 37*((int)(gValue1)+(int)(gValue2)) + 56*((int)(bValue1)+(int)(bValue2))) >> 9) + 64;
                    *vAddr++ = (( 56*((int)(rValue1)+(int)(rValue2)) - 47*((int)(gValue1)+(int)(gValue2)) -  9*((int)(bValue1)+(int)(bValue2))) >> 9) + 64;
                    
                    rAddr += pixelStr2;
                    gAddr += pixelStr2;
                    bAddr += pixelStr2;
                }
            } else {
                for (int w = 0; w < widthHalf; w++ )
                {
                    unsigned char const rValue1 = *rAddr;
                    unsigned char const gValue1 = *gAddr;
                    unsigned char const bValue1 = *bAddr;

                    unsigned char const rValue2 = *(rAddr + pixelStride);
                    unsigned char const gValue2 = *(gAddr + pixelStride);
                    unsigned char const bValue2 = *(bAddr + pixelStride);

                    *yAddr++ = (( 66*rValue1 + 129*gValue1 + 25*bValue1) >> 8) + 16;
                    *yAddr++ = (( 66*rValue2 + 129*gValue2 + 25*bValue2) >> 8) + 16;
                    *uAddr++ += ((-19*((int)(rValue1)+(int)(rValue2)) - 37*((int)(gValue1)+(int)(gValue2)) + 56*((int)(bValue1)+(int)(bValue2))) >> 9) + 64;
                    *vAddr++ += (( 56*((int)(rValue1)+(int)(rValue2)) - 47*((int)(gValue1)+(int)(gValue2)) -  9*((int)(bValue1)+(int)(bValue2))) >> 9) + 64;
                    
                    rAddr += pixelStr2;
                    gAddr += pixelStr2;
                    bAddr += pixelStr2;
                }
            }
            bytesRowStart += scanlineStride;
        }
        return true;
    }
    catch(...) 
    {
        return false;
    }
}

bool convertToYUV444(unsigned char const * const rgbData, 
                     int width, int height,
                     int rOffset, int gOffset, int bOffset, 
                     int pixelStride, int scanlineStride,
                     uint8_t * const yuvPlanes[3])
{
    // This RGB to YUV conversion is based on the same matrix as
    // https://github.com/camilleg/unblock/blob/master/main.cpp#L19
    // and http://www.equasys.de/colorconversion.html YCbCr - RGB.
    // 
    // which is
    // 
    // |  0.257 0.504 0.098 |   | R |   | 16  |
    // | -0.148 0.291 0.439 | X | G | + | 128 |
    // |  0.439 0.368 0.071 |   | B |   | 128 |
    // 
    // The corresponding non-floating point operations implemented below are
    // 
    // |  66  129   25 |   | R |               |  16 |
    // | -38  -74  112 | X | G | X (1 / 256) + | 128 |
    // | 112  -94  -18 |   | B |               | 128 |
    // 
    // Note that the client will have to implement the correct inverse 
    // transformation to preserve color fidelity across the client and server

    try 
    {
        unsigned char const * bytesStart = rgbData;
        uint8_t * y_addr = yuvPlanes[0];
        uint8_t * u_addr = yuvPlanes[1];
        uint8_t * v_addr = yuvPlanes[2];

        for (int h = 0; h < height; h++)
        {
            unsigned char const * r_addr = bytesStart + rOffset;
            unsigned char const * g_addr = bytesStart + gOffset;
            unsigned char const * b_addr = bytesStart + bOffset;

            for (int w = 0; w < width; w++)
            {
                // cache RGB values for matrix multiplication
                unsigned char const rValue = *r_addr;
                unsigned char const gValue = *g_addr;
                unsigned char const bValue = *b_addr;

                // convert RGB to YUV
                *y_addr++ = (( 66*rValue + 129*gValue + 25*bValue) >> 8) + 16;
                *u_addr++ = ((-19*rValue -  37*gValue + 56*bValue) >> 7) + 128;
                *v_addr++ = (( 56*rValue -  47*gValue -  9*bValue) >> 7) + 128;

                // move to next RGB pixel
                r_addr += pixelStride;
                g_addr += pixelStride;
                b_addr += pixelStride;
            }

            // move to next line in RGB data
            bytesStart += scanlineStride;
        }

        return true;
    }
    catch(...) 
    {
        return false;
    }
}
