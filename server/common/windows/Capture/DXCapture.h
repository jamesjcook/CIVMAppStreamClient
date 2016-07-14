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

#pragma warning (push)

#if _MSC_VER && !__INTEL_COMPILER
    // Disable warnings about macro re-definitions if we're using the DX SDK
    // http://stackoverflow.com/questions/19336841/i-get-a-lot-of-macro-redefinition-when-using-directx-10
    #pragma warning (disable: 4005)
#endif

#include <d3d11.h>

#pragma warning (pop)

#include "../Capture/IScreenCapture.h"

/**
 * @class   DXCapture
 *
 * @brief   Captures framebuffer data given information about a Direct3D 11 device and its 
 *          back buffer.
 *
 */

class DXCapture: public IScreenCapture
{
private:
    // Hide default constructor, copy-constructor and assignment operator
    DXCapture() {}
    DXCapture(DXCapture const&) {}
    DXCapture& operator=(DXCapture const&) {}

    // Fields to remember state for the capture method
    ID3D11Device* _d3D11Device;
    ID3D11DeviceContext* _d3D11DeviceContext;
    ID3D11Texture2D* _stagingBuffer;
    ID3D11Texture2D* _backbuffer;
    int _backbufferWidth;
    int _backbufferHeight;

public:

    /**
     * @fn  DXCapture::DXCapture(ID3D11Device* d3DDevice, ID3D11DeviceContext* d3DDeviceContext, ID3D11Texture2D* backbuffer, int backbufferWidth, int backbufferHeight);
     *
     * @brief   Construct the DXCapture object and set up any state required
     *
     * @param [in,out]  d3DDevice           A pointer to a valid Direct3D 11 device
     * @param [in,out]  d3DDeviceContext    A pointer to a valid Direct3D 11 device context
     * @param [in,out]  backbuffer          A pointer to the backbuffer of the supplied Direct3D device
     * @param   backbufferWidth             Width of the backbuffer
     * @param   backbufferHeight            Height of the backbuffer
     */

    DXCapture(ID3D11Device* d3DDevice, ID3D11DeviceContext* d3DDeviceContext, ID3D11Texture2D* backbuffer, int backbufferWidth, int backbufferHeight);

    /**
     * @fn  DXCapture::~DXCapture();
     *
     * @brief   Destructor, cleans up after we're done using this object
     *
     */

    ~DXCapture();

    /**
     * @fn  bool DXCapture::supported();
     *
     * @brief   Tells us if this capture method is supported
     *
     *
     * @return  true if it is, false otherwise
     */

    bool supported();

    /**
     * @fn  const unsigned char* DXCapture::capture();
     *
     * @brief   Synchronously captures the image and returns a pointer to raw RGB data in the format 
     *          denoted by @see DXCapture::getCapturePixelFormat
     *
     * @return  A valid pointer to RGB color data or NULL if it fails
     */

    const unsigned char* capture();

    /**
     * @fn  void DXCapture::endCapture();
     *
     * @brief   Ends a capture. This instructs this capture method to release any resources
     *          associated with the capture of this frame. This method has to be called
     *          when the caller is done with the captured frame data otherwise
     *          subsequent captures will fail
     *
     */

    void endCapture();

    /**
     * @fn  CapturePixelFormat DXCapture::getCapturePixelFormat();
     *
     * @brief   Gets the capture pixel format as values of the enumeration @see CapturePixelFormat
     *
     * @return  The actual pixel format of the data returned by @see DXCapture::capture
     */

    CapturePixelFormat getCapturePixelFormat();
};
