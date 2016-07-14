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


#ifndef _included_OGLRenderer_h
#define _included_OGLRenderer_h

#include "VideoRenderer.h"

#include "XStx/common/XStxAPI.h"

#include "Config.h"

#if GL_USES_OPENGLES
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#elif GL_USES_GLES2
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#elif GL_USES_OPENGL
#include <OpenGL/gl.h>
#endif


#undef LOG_TAG
#define LOG_TAG "OGLRenderer.h"
#include "log.h"

/**
 * An OpenGL implementation of VideoRenderer, specifically for Android.
 */
class OGLRenderer : public VideoRenderer
{
public:
    /**
     * Constructor.
     */
    OGLRenderer();
    /**
     * Destructor.
     */
    virtual ~OGLRenderer();

    virtual bool init();
    virtual void render();
    virtual int draw();
    virtual void requestFrame();
    virtual void pause(bool pause);
    virtual void setViewport(int x, int y, int w, int h);
    virtual void setKeyboardOffset(int offset);
    virtual void clearScreen();
    virtual void setDisplayDimensions(uint32_t w, uint32_t h, bool forceRescale)
    {
        if (!forceRescale && mScaled)
        {
            LOGV("rescaling : %d, %d, %d", mKeyboardOffset, h, mDisplayHeight);
            if (h>mDisplayHeight)
            {
                mBaseKeyboardOffset = 0;
            }
            else
            {
                mBaseKeyboardOffset = h - mDisplayHeight;
            }
        }
        VideoRenderer::setDisplayDimensions(w, h, forceRescale);
    }

    virtual void getScaleAndOffset(float &scale, int &x, int &y)
    {
        VideoRenderer::getScaleAndOffset(scale, x, y);
        y += mKeyboardOffset ;
    }

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

protected:
    void glInit();

    /**
     * The type of decode to expect. FFMPEG delivers planes; some other
     * decoders interleave the YUV components.
     */
    VideoDecoder::EDecodeType mActiveDecodeType;

    /**
     * True when we're paused. When paused, no OpenGL calls should happen.
     * When we unpause, we need to reinitialize the OpenGL state.
     */
    bool mPaused;

    /**
     * The OpenGL vertex/pixel shader program.
     */
    GLuint mProgram;

    /**
     * The handle for the vertex position.
     */
    GLuint mvPositionHandle;

    /**
     * The handle for the vertex texture coordinate.
     */
    GLuint mvVertexTexCoord;

    /**
     * The three (YUV) texture handles for use in the pixel shader script.
     */
    int mTexUniform[3];

    /**
     * A texture coordinate array: Can be offset from the corners if
     * we don't want to display the entire frame. (i.e., texture
     * width < display width, or similarly for height).
     */
    GLfloat mTextureCoords[8];

    /**
     * A double-buffer of the three planes -- Y, U, and V -- to make six
     * textures, total.
     */
    GLuint mTextures[6];

    /**
     * Texture widths for the corresponding mTextures[]. When zero, it means
     * the corresponding texture hasn't been allocated from OpenGL.
     */
    int mAllocatedTextureWidths[6];

    /**
     * The right edge that we want to display. Used to calculate the
     * U texture coordinates.
     */
    GLfloat mNewRightEdgeCoord;
    /**
     * The bottom edge that we want to display. Used to calculate
     * the V texture coordinates.
     */
    GLfloat mNewBotEdgeCoord;

    /**
     * An offset applied to the glViewport() that moves the screen aside
     * for the keyboard. Calculated based on how much the viewport size
     * shrinks.
     */
    int mBaseKeyboardOffset;

    /**
     * An scrolling offset applied to the glViewport() that moves the screen
     * around as the user drags.
     */
    int mKeyboardOffset;

    /**
     * Current Chroma sampling mode.
     */
    XStxChromaSampling mChromaSampling;

    /**
     * A helper for logging GL error messages.
     *
     * @param name A description of where the error happened.
     * @param s    A GL error code.
     */
    void printGLString(const char *name, GLenum s);

    void createTexture(int textureId);
    void bindAndPopulateTexture(int textureToggle, int id, int uniformLocation, GLubyte *textureData, int bufferWidth, int bufferHeight);
    GLuint loadShader(GLenum shaderType, const char *pSource);
    GLuint createProgram(const char *pVertexSource, const char *pFragmentSource);

};

#endif
