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

#include "Windows.h"
#include "IScreenCapture.h"

/**
 * @class   GDICapture
 *
 * @brief   GDI Capture method. This is most compatible, but slower than 
 *          hardware-accelerated capture methods
 *
 * @ingroup XStxSampleVDIServer
 */

class GDICapture: public IScreenCapture
{
private:
    HDC _windowDC;
    int _windowWidth;
    int _windowHeight;
    int _windowDCBpp;
    HWND _windowHandle;
    __declspec(align(16)) unsigned char* _pixels;
    HDC _memDC;
    HBITMAP _compatibleBitmap;

    void getWindowDimensions(HWND hWnd, int& horizontal, int& vertical);
    int getBitsPerPixel(HWND hWnd);
    bool captureInternal(HDC hdcScreen, HDC hdcMemDC, HBITMAP hbmScreen, 
                         unsigned char* lpbitmap, bool save = false);

protected:
    
    // Hide copy constructor and assignment operator
    GDICapture(const GDICapture&) {};
    GDICapture& operator=(const GDICapture&) {};

public:

    /**
     * @fn  GDICapture::GDICapture();
     *
     * @brief   Default constructor.
     *
     */

    GDICapture(HWND hWnd = NULL);

    /**
     * @fn  bool GDICapture::supported();
     *
     * @brief   Checks to see if this capture method is supported
     *
     * @return  true if it is, false otherwise.
     */

    bool supported();

    /**
     * @fn  const unsigned char* GDICapture::capture();
     *
     * @brief   Captures the current desktop and returns bytes containing the image
     *          in the pixelformat indicated by  @see GDICapture::getCapturePixelFormat
     *
     * @return  null if it fails, else a char*.
     */

    const unsigned char* capture();

    /**
     * @fn  CapturePixelFormat GDICapture::getCapturePixelFormat();
     *
     * @brief   Gets the pixel format of this current capture method
     *
     * @return  The capture pixel format @see ::CapturePixelFormat
     */

    CapturePixelFormat getCapturePixelFormat();
    ~GDICapture();
};
