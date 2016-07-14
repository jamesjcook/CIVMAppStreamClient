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


#ifndef __AppStreamSampleClient__VDAOGLRenderer__
#define __AppStreamSampleClient__VDAOGLRenderer__

#include "OGLRenderer.h"

/**
 * An implementation of OGLRenderer for the OSX VDADecoder.
 */
class VDAOGLRenderer : public OGLRenderer
{
public:
    /**
     * Constructor.
     */
    VDAOGLRenderer();

    virtual int draw();

    virtual void render();

    /**
     * Creates the texture from the passed in imageBuffer
     *
     * @param[in] imageBuffer A CVImageBufferRef for the video frame to be rendered next
     */
    virtual void setCVPixelBuffer(CVImageBufferRef imageBuffer);

    /**
     * Whether we should use VDADecoder or if we need to fall back to FFmpeg
     */
    BOOL            _useVDADecoder;
    
    /**
     * Pass general configuration parameters to decoder
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool receivedClientConfiguration(const XStxClientConfiguration *config);
    /**
     * Provide Chroma sampling capability
     * TODO: will be made pure virtual once each client platform implements this.
     */
    virtual bool isChromaSamplingSupported(XStxChromaSampling chromaSampling);


private:
    virtual VideoDecoder::EDecodeType getDecodeType()
    {
        return VideoDecoder::DECODE_PLANES;
    }
    /**
     * Creates a texture from an IOSurfaceRef
     */
    void bindAndPopulateTexture(IOSurfaceRef theIOSurface);


    /**
     * Whether or not we have a surface texture to render
     */
    BOOL            _haveIOSurface;

    /**
     * The handle for the video texture
     */
    GLuint          _videoTexture;

    /**
     * Width of the video texture
     */
    size_t          _videoWidth;

    /**
     * Height of the video texture
     */
    size_t          _videoHeight;

    /**
     * Reference to the CGL Context
     */
    CGLContextObj   _cgl_ctx;
};

#endif /* defined(__AppStreamSampleClient__VDAOGLRenderer__) */
