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

#include "XStx/common/XStxAPI.h"

/**
 * @enum    CapturePixelFormat
 *
 * @brief   Enumeration indicating the pixelformat returned by @see IScreenCapture::capture.
 */

enum CapturePixelFormat
{
    CAPTURE_PIXELFORMAT_UNKNOWN,
    CAPTURE_PIXELFORMAT_R8G8B8A8,
    CAPTURE_PIXELFORMAT_YUV420,
    CAPTURE_PIXELFORMAT_B8G8R8A8,
    CAPTURE_PIXELFORMAT_B8G8R8,
};

/**
 * @class   IScreenCapture
 *
 * @brief   Interface that provides methods for capturing images of the desktop and checking
 *          support/pixelformats.
 *
 */

class IScreenCapture
{
public:

    /**
     * @fn  ~IScreenCapture()
     *
     * @brief   Destructor.
     *
     */

    ~IScreenCapture() {}

    /**
     * @fn  virtual bool supported() = 0;
     *
     * @brief   This method checks to see if the capture method is supported
     *
     * @return  true if it is, false otherwise.
     */

    virtual bool supported() = 0;

    /**
     * @fn  virtual const unsigned char* capture() = 0;
     *
     * @brief   Captures the current desktop and returns bytes containing the image in the
     *          pixelformat indicated by  @see IScreenCapture::getCapturePixelFormat.
     *
     * @return  null if it fails, else a char*.
     */

    virtual const unsigned char* capture() = 0;

    /**
     * @fn  virtual CapturePixelFormat getCapturePixelFormat() = 0;
     *
     * @brief   Gets the pixel format of this current capture method.
     *
     * @return  The capture pixel format @see ::CapturePixelFormat.
     */

    virtual CapturePixelFormat getCapturePixelFormat() = 0;

    /**
     * @fn  virtual void IScreenCapture::endCapture()
     *
     * @brief   Ends a capture, letting the capture method know that
     *          any per-frame resources can now be released
     *
     */

    virtual void endCapture() {}
};
