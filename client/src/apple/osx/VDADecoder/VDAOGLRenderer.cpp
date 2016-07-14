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


#include "VDAOGLRenderer.h"

#undef LOG_TAG
#define LOG_TAG "VDAOGLRenderer"
#include "log.h"

#include "platformBindings.h"


static const GLfloat gTriangleVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat gTextureCoordsPortrait[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

VDAOGLRenderer::VDAOGLRenderer() :
_useVDADecoder(true)
{
}


static void checkGlError(const char *op)
{
#if 0
    for (GLint error = glGetError(); error; error
         = glGetError())
    {
        LOGE("after %s() glError (0x%x)\n", op, error);
    }
#endif
}

void VDAOGLRenderer::render()
{
    if (!_useVDADecoder) {
        //Not using hardware so fall back to OGLRenderer
        OGLRenderer::render();
        return;
    }
    if (mPaused)
    {
        return;
    }

    //If we queued up video frames this is where we should deque and render them
    // but we don't queue frames - we just render the newest frame we have
}

int VDAOGLRenderer::draw()
{
    if (!_useVDADecoder) {
        //Not using hardware so fall back to OGLRenderer
        return OGLRenderer::draw();
    }
    if (mPaused)
    {
        return 0;
    }

    //Store the CGL Context for use in bindAndPopulate
    _cgl_ctx = CGLGetCurrentContext();

    glViewport(0,0,mDisplayWidth,mDisplayHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(mWidthOffset, mHeightOffset+mKeyboardOffset, mWidth, mHeight);

    if (mDecodeType == VideoDecoder::DECODE_UNDEFINED)
    {
        LOGW("Decoder type undefined in draw!!");
        mFrameValid = false;
    }

    // Check to see if the queue has a frame in it, and if so, render it.
    int frameCount = checkQueue();


    //Clear background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (_haveIOSurface) {

        GLint		origMatrixMode;

        //ARB textures need texture coordinates based on their size
        // this matrix normalizes it to 0-1 similar to a standard texture
        glGetIntegerv(GL_MATRIX_MODE, &origMatrixMode);
        glMatrixMode(GL_TEXTURE);
        glPushMatrix();
        glScalef(_videoWidth, _videoHeight, 1.0);

        //Load the actual texture
        glEnable(GL_TEXTURE_RECTANGLE_ARB);
        glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _videoTexture);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

        //Render the video quad
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glVertexPointer(2, GL_FLOAT, 0, gTriangleVertices);
        glTexCoordPointer(2, GL_FLOAT, 0, gTextureCoordsPortrait);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);

        glDisable(GL_TEXTURE_RECTANGLE_ARB);

        //Put the matrix mode back and pop our texture scaling matrix
        glMatrixMode(GL_TEXTURE);
        glPopMatrix();
        glMatrixMode(origMatrixMode);

    } else {
        //No video yet
        glClear(GL_COLOR_BUFFER_BIT);
    }

    return frameCount;
}

void VDAOGLRenderer::bindAndPopulateTexture(IOSurfaceRef theIOSurface)
{
    //Make sure we have a surface
    if (theIOSurface != nil) {
        //Make sure we have an OpenGL context
        if (_cgl_ctx != NULL) {
            CGLSetCurrentContext(_cgl_ctx);

            if (!_videoTexture) {
                //Create the videoTexture
                glEnable(GL_TEXTURE_RECTANGLE_ARB);
                checkGlError("glEnable GL_TEXTURE_RECTANGLE_ARB");
                glGenTextures(1, &_videoTexture);
                checkGlError("glGenTextures 1_videoTexture");
                glDisable(GL_TEXTURE_RECTANGLE_ARB);
                checkGlError("glDisable GL_TEXTURE_RECTANGLE_ARB");
            }


            glEnable(GL_TEXTURE_RECTANGLE_ARB);
            checkGlError("glEnable GL_TEXTURE_RECTANGLE_ARB");
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, _videoTexture);
            checkGlError("glBindTexture GL_TEXTURE_RECTANGLE_ARB");

            _videoWidth = IOSurfaceGetWidth(theIOSurface);
            _videoHeight = IOSurfaceGetHeight(theIOSurface);

            CGLError gotError = CGLTexImageIOSurface2D(_cgl_ctx, GL_TEXTURE_RECTANGLE_ARB, GL_RGB8,
                                   (GLsizei)_videoWidth, (GLsizei)_videoHeight,
                                   GL_YCBCR_422_APPLE, GL_UNSIGNED_SHORT_8_8_APPLE,
                                   theIOSurface, 0);
            if (gotError) {
                NSLog(@"Got CGLError");
            }
            checkGlError("glBindTexture GL_TEXTURE_RECTANGLE_ARB");
            glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);
            glDisable(GL_TEXTURE_RECTANGLE_ARB);

            glFlush();

            _haveIOSurface = true;
        }
    }
}


void VDAOGLRenderer::setCVPixelBuffer(CVImageBufferRef imageBuffer)
{
    //On retina MacBooks the pixel buffer is overwritten by the decoder
    // as a temporary workaround we copy this into our own buffer ensuring
    // we always render the proper frame

    static CVPixelBufferRef _pixelBuffers[2];
    static int currBuffer = 0;

    if (!_pixelBuffers[0]) {
        //Create the attributes dict so we can make it iOSurface backed
        NSMutableDictionary *attributes = [NSMutableDictionary dictionary];
        [attributes setObject:[NSDictionary dictionary] forKey:(NSString*)kCVPixelBufferIOSurfacePropertiesKey];

        //Create 2 pixel buffers to toggle between
        CVPixelBufferCreate(kCFAllocatorDefault, CVPixelBufferGetWidth(imageBuffer), CVPixelBufferGetHeight(imageBuffer), CVPixelBufferGetPixelFormatType(imageBuffer), (__bridge CFDictionaryRef)attributes, &_pixelBuffers[0]);

        CVPixelBufferCreate(kCFAllocatorDefault, CVPixelBufferGetWidth(imageBuffer), CVPixelBufferGetHeight(imageBuffer), CVPixelBufferGetPixelFormatType(imageBuffer), (__bridge CFDictionaryRef)attributes, &_pixelBuffers[1]);
    }

    //Toggle the current pixel buffer
    currBuffer ^= 1;

    //Lock both pixelBuffers
    CVPixelBufferLockBaseAddress(imageBuffer, 0);

    CVPixelBufferLockBaseAddress(_pixelBuffers[currBuffer], 0);

    //Copy the data into our new buffer
    unsigned char *origBuffer = (unsigned char*)CVPixelBufferGetBaseAddress(imageBuffer);
    unsigned char *newBuffer = (unsigned char *)CVPixelBufferGetBaseAddress(_pixelBuffers[currBuffer]);

    memcpy(newBuffer, origBuffer, CVPixelBufferGetDataSize(imageBuffer));

    //Unlock the incoming buffer
    CVPixelBufferUnlockBaseAddress(imageBuffer, 0);

    //Get the IOSurface from the CVBuffer
    IOSurfaceRef theIOSurfaceRef = CVPixelBufferGetIOSurface(_pixelBuffers[currBuffer]);

    //Bind it to a texture
    bindAndPopulateTexture(theIOSurfaceRef);

    //Unlock our buffer
    CVPixelBufferUnlockBaseAddress(_pixelBuffers[currBuffer], 0);
}

/**
 * Provide Chroma sampling capability
 */
bool VDAOGLRenderer::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
{
    // For the basic OpenGL renderer, 420 and 444 should always be supported.
    // If a particular platform's renderer doesn't support it, you should
    // handle that in your platform's derived renderer class.
    return chromaSampling == XSTX_CHROMA_SAMPLING_YUV420
    || chromaSampling == XSTX_CHROMA_SAMPLING_YUV444;
}

/**
 * Pass general configuration parameters to decoder
 */
bool VDAOGLRenderer::receivedClientConfiguration(const XStxClientConfiguration *config)
{
    // YUV420 or YUV444
    mChromaSampling = config->mChromaSampling;
    
    // Don't override PBuffer; PBuffer means that something else is handling the decode.
    if (mChromaSampling==XSTX_CHROMA_SAMPLING_YUV444 && mDecodeType!=VideoDecoder::DECODE_PBUFFER)
    {
        mDecodeType = VideoDecoder::DECODE_PLANES444;
    }
    
    // no log ?? LOG("Received chroma sampling option: %d", static_cast<int>(mChromaSampling));
    return isChromaSamplingSupported(mChromaSampling);
}
