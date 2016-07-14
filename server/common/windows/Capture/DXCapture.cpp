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

#include "DXCapture.h"

DXCapture::DXCapture(ID3D11Device* d3DDevice, ID3D11DeviceContext* d3DDeviceContext, 
                     ID3D11Texture2D* backbuffer, 
                     int backbufferWidth, int backbufferHeight)
    : _d3D11Device(d3DDevice),
      _d3D11DeviceContext(d3DDeviceContext),
      _backbuffer(backbuffer),
      _backbufferWidth(backbufferWidth),
      _backbufferHeight(backbufferHeight)
{
}

bool DXCapture::supported()
{
    try
    {
        // Create a staging resource
        D3D11_TEXTURE2D_DESC stagingTextureDesc;
        stagingTextureDesc.Width = _backbufferWidth;
        stagingTextureDesc.Height = _backbufferHeight;
        stagingTextureDesc.MipLevels = stagingTextureDesc.ArraySize = 1;
        stagingTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        stagingTextureDesc.SampleDesc.Count = 1;
        stagingTextureDesc.SampleDesc.Quality = 0;
        stagingTextureDesc.Usage = D3D11_USAGE_STAGING;
        stagingTextureDesc.BindFlags = 0;
        stagingTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
        stagingTextureDesc.MiscFlags = 0;

        if (!SUCCEEDED(_d3D11Device->CreateTexture2D(&stagingTextureDesc, NULL, &_stagingBuffer)))
            return false;
        
        return true;
    }
    catch (...)
    {
        return false; // Something went wrong, we can't set up DX capture
    }
}

CapturePixelFormat DXCapture::getCapturePixelFormat()
{
    return CAPTURE_PIXELFORMAT_R8G8B8A8; // Always
}

const unsigned char* DXCapture::capture()
{
    // Copy the frame to the staging buffer
    _d3D11DeviceContext->CopyResource(_stagingBuffer, _backbuffer);

    // Lock the staging buffer and post it to the host app for encoding
    D3D11_MAPPED_SUBRESOURCE mappedBuffer;
    HRESULT bufferMapResult = _d3D11DeviceContext->Map(_stagingBuffer, 0, D3D11_MAP_READ, 0, &mappedBuffer);
    if (SUCCEEDED(bufferMapResult))
    {
        unsigned char* pixel_bytes = (unsigned char*)mappedBuffer.pData;
        return pixel_bytes;
    }

    return NULL;
}

void DXCapture::endCapture()
{
    // Unmap this buffer, the application is done with the capture
    _d3D11DeviceContext->Unmap(_stagingBuffer, 0);
}

DXCapture::~DXCapture()
{
    if (_stagingBuffer != NULL)
    {
        _stagingBuffer->Release();
        _stagingBuffer = NULL;
    }
}
