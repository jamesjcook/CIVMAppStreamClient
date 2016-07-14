/*
 * NOTICE: Amazon has modified this file from the original.
 * Modifications Copyright (C) 2013-2014 Amazon.com, Inc. or its
 * affiliates. All Rights Reserved.
 *
 * Modifications are licensed under the Amazon Software License
 * (the "License"). You may not use this file except in compliance
 * with the License. A copy of the License is located at
 *
 *   http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 *  OR CONDITIONS OF ANY KIND, express or implied. See the License for
 *  the specific language governing permissions and limitations under the License.
 *
 * Original source:
 *   https://developer.android.com/tools/sdk/ndk/index.html
 */

/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */


#include "OGLRenderer.h"

#undef LOG_TAG
#define LOG_TAG "OGLRenderer"
#include "log.h"

// Set to one to display GL errors
#define LOG_GL_ERRORS 0

#include "platformBindings.h"

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <MUD/base/TimeVal.h>

static const GLfloat gTriangleVertices[] = { -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f };
static const GLfloat gTextureCoordsPortrait[] = { 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

// If we add rotation back, use this
//static const GLfloat gTextureCoordsLandscapeLeft[] = { 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f };
//static const GLfloat gTextureCoordsLandscapeRight[] = { 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f };

OGLRenderer::OGLRenderer() :
    mPaused(false),
    mActiveDecodeType(VideoDecoder::DECODE_UNDEFINED),
    mProgram(0),
    mvPositionHandle(0),
    mvVertexTexCoord(0),
    mNewRightEdgeCoord(0),
    mNewBotEdgeCoord(0),
    mKeyboardOffset(0),
    mBaseKeyboardOffset(0),
    mChromaSampling(XSTX_CHROMA_SAMPLING_UNKNOWN)
{
    memcpy(mTextureCoords, gTextureCoordsPortrait, sizeof(GLfloat) * 8);
}
OGLRenderer::~OGLRenderer()
{
    LOGW("OGLRenderer::~OGLRenderer");
}


static void checkGlError(const char *op)
{
#if LOG_GL_ERRORS
    for (GLint error = glGetError(); error; error
         = glGetError())
    {
        LOGE("after %s() glError (0x%x)\n", op, error);
    }
#endif
}

void OGLRenderer::printGLString(const char *name, GLenum s)
{
    const char *v = (const char *)glGetString(s);
    if (v == 0)
    {
        LOGI("Error retrieving GLString for %s", name);
        checkGlError("glGetString");
    }
    else
    {
        LOGI("GL %s = %s\n", name, v);
    }

}



static const char gVertexShader[] =
    "attribute vec4 vPosition;\n"
    "attribute vec2 vertexTexCoord;\n"
    "\n"
    "varying vec2 texCoord;\n"
    "\n"
    "void main() {\n"
    "  gl_Position = vPosition;\n"
    "  texCoord = vertexTexCoord;"
    "}\n";


//OpenGL does not have precision mediump float
static const char gFragmentShader[] =
    "uniform sampler2D Ytex;\n"
    "uniform sampler2D Utex, Vtex;\n"
#if GL_USES_OPENGLES || GL_USES_GLES2
    "precision mediump float;\n"
#endif
    "varying vec2 texCoord;\n"
    "void main() {\n"
    "  float r,g,b,y,u,v;\n"
    "  y=texture2D(Ytex,texCoord).r;\n"
    "  u=texture2D(Utex,texCoord).r;\n"
    "  v=texture2D(Vtex,texCoord).r;\n"

    "  y=1.1643*(y-0.0625);\n"
    "  u=u-0.5;\n"
    "  v=v-0.5;\n"

    "  r=y+1.5958*v;\n"
    "  g=y-0.39173*u-0.81290*v;\n"
    "  b=y+2.017*u;\n"

    "  gl_FragColor = vec4(r, g, b, 1.0);\n"
    "}\n";



static const char gVertexShaderI[] =
    "attribute vec4 vPosition;\n"
    "attribute vec2 vertexTexCoord;\n"
    "\n"
    "varying vec2 texCoord;\n"
    "varying vec2 UVtexCoord;\n"
    "vec2 scale;\n"
    "\n"
    "void main() {\n"
    "  scale = vec2(0.5,1);\n"
    "  gl_Position = vPosition;\n"
    "  texCoord = vertexTexCoord;"
    "  UVtexCoord = vertexTexCoord*scale;"
    "}\n";

static const char gFragmentShaderI[] =
    "uniform sampler2D Ytex;\n"
    "uniform sampler2D Utex, Vtex;\n"
    "precision mediump float;\n"
    "varying vec2 texCoord;\n"
    "varying vec2 UVtexCoord;\n"
    "void main() {\n"
    "  float r,g,b,y,u,v;\n"
    "  y=texture2D(Ytex,texCoord).r;\n"
    "  u=texture2D(Utex,UVtexCoord).r;\n"
    "  v=texture2D(Vtex,UVtexCoord).r;\n"

    "  y=1.1643*(y-0.0625);\n"
    "  u=u-0.5;\n"
    "  v=v-0.5;\n"

    "  r=y+1.5958*v;\n"
    "  g=y-0.39173*u-0.81290*v;\n"
    "  b=y+2.017*u;\n"

    "  gl_FragColor = vec4(r, g, b, 1.0);\n"
    "}\n";

static const char gFragmentShaderP[] =
    "#extension GL_OES_EGL_image_external : require\n"

    "precision mediump float;\n"
    "varying vec2 texCoord;\n"
    "uniform samplerExternalOES sTexture;\n"

    "void main() {\n"
    "  gl_FragColor = texture2D(sTexture, texCoord);\n"
    "}\n";

GLuint OGLRenderer::loadShader(GLenum shaderType, const char *pSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader)
    {
        glShaderSource(shader, 1, &pSource, NULL);
        glCompileShader(shader);
        GLint compiled = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if (!compiled)
        {
            GLint infoLen = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
            if (infoLen)
            {
                char *buf = new char[infoLen];
                if (buf)
                {
                    glGetShaderInfoLog(shader, infoLen, NULL, buf);
                    LOGE("Could not compile shader %d:\n%s\n",
                         shaderType, buf);
                    delete[] buf;
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
    }
    return shader;
}

GLuint OGLRenderer::createProgram(const char *pVertexSource, const char *pFragmentSource)
{
    GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
    if (!vertexShader)
    {
        return 0;
    }

    GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
    if (!pixelShader)
    {
        return 0;
    }

    GLuint program = glCreateProgram();
    if (program)
    {
        glAttachShader(program, vertexShader);
        checkGlError("glAttachShader");
        glAttachShader(program, pixelShader);
        checkGlError("glAttachShader");
        glLinkProgram(program);
        GLint linkStatus = GL_FALSE;
        glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
        if (linkStatus != GL_TRUE)
        {
            GLint bufLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
            if (bufLength)
            {
                char *buf = new char[bufLength];
                if (buf)
                {
                    glGetProgramInfoLog(program, bufLength, NULL, buf);
                    LOGE("Could not link program:\n%s\n", buf);
                    delete[] buf;
                }
            }
            glDeleteProgram(program);
            program = 0;
        }
    }
    return program;
}

void OGLRenderer::createTexture(int textureId)
{
    int textureType = GL_TEXTURE_2D;

    mAllocatedTextureWidths[textureId] = 0;
    glActiveTexture(GL_TEXTURE0 + textureId);
    checkGlError("glActiveTexture");

    if (mDecodeType == VideoDecoder::DECODE_PBUFFER)
    {
#if ANDROID
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[textureId]);
        textureType = GL_TEXTURE_EXTERNAL_OES;
#else
        assert(false);
#endif
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, mTextures[textureId]);
    }

    glTexParameteri(textureType, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    checkGlError("glTexParameteri GL_TEXTURE_MAG_FILTER");
    glTexParameteri(textureType, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    checkGlError("glTexParameteri GL_TEXTURE_MIN_FILTER");
    glTexParameteri(textureType, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    checkGlError("glTexParameteri GL_TEXTURE_WRAP_S");
    glTexParameteri(textureType, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    checkGlError("glTexParameteri GL_TEXTURE_WRAP_T");
}

bool OGLRenderer::init()
{
    LOGI("OGLRenderer::init()");

    printGLString("Version", GL_VERSION);
    printGLString("Vendor", GL_VENDOR);
    printGLString("Renderer", GL_RENDERER);
    printGLString("Extensions", GL_EXTENSIONS);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    return true;
}

void OGLRenderer::setViewport(int x, int y, int w, int h)
{
    VideoRenderer::setViewport(x, y, w, h);
    glViewport(mWidthOffset, mHeightOffset, mWidth, mHeight);
}

void OGLRenderer::bindAndPopulateTexture(int textureToggle, int id, int uniformLocation, GLubyte *textureData, int bufferWidth, int bufferHeight)
{
    if (textureToggle != 0)
    {
        id += 3;
    }
    glActiveTexture(GL_TEXTURE0 + id);
    checkGlError("glActiveTexture");
    glBindTexture(GL_TEXTURE_2D, mTextures[id]);
    checkGlError("glBindTexture");
    glUniform1i(uniformLocation, id);
    checkGlError("glUniform1i");

    if (mAllocatedTextureWidths[id] == 0 || mAllocatedTextureWidths[id] != bufferWidth)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, bufferWidth, bufferHeight, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData);
        checkGlError("glTexImage2D");
        mAllocatedTextureWidths[id] = bufferWidth;
    }
    else
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, bufferWidth, bufferHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, textureData);
        checkGlError("glTexSubImage2D");
    }
}

void OGLRenderer::glInit()
{
    LOGV("glInit()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mActiveDecodeType = mDecodeType;
    if (mProgram)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }

    if (mDecodeType == VideoDecoder::DECODE_INTERLEAVED)
    {
        mProgram = createProgram(gVertexShaderI, gFragmentShaderI);
    }
    else if (mDecodeType == VideoDecoder::DECODE_PBUFFER)
    {
        mProgram = createProgram(gVertexShader, gFragmentShaderP);
    }
    else // PLANES && PLANES444
    {
        mProgram = createProgram(gVertexShader, gFragmentShader);
    }
    if (!mProgram)
    {
        LOGE("Could not create program.");
        mvPositionHandle = 0;
        mvVertexTexCoord = 0;
        return;
    }

    mvPositionHandle = glGetAttribLocation(mProgram, "vPosition");
    checkGlError("glGetAttribLocation(mProgram, 'vPosition');");
    mvVertexTexCoord = glGetAttribLocation(mProgram, "vertexTexCoord");
    checkGlError("glGetAttribLocation(mProgram, 'vertexTexCoord');");

    glUseProgram(mProgram);
    checkGlError("glUseProgram");

    if (mDecodeType == VideoDecoder::DECODE_PBUFFER)
    {
        // find variable locations inside our shader
        mTexUniform[0] = glGetUniformLocation(mProgram, "sTexture");

        // create the single mTexture
        glGenTextures(1, mTextures);
        createTexture(0);

        // Bind it to the hardware renderer
        mTextures[0] = platformBindVideoTexture(mTextures[0]);
    }
    else
    {
        // find variable locations inside our shader
        mTexUniform[0] = glGetUniformLocation(mProgram, "Ytex");
        checkGlError("glGetUniformLocation 0");
        mTexUniform[1] = glGetUniformLocation(mProgram, "Utex");
        checkGlError("glGetUniformLocation 1");
        mTexUniform[2] = glGetUniformLocation(mProgram, "Vtex");
        checkGlError("glGetUniformLocation 2");


        glGenTextures(6, mTextures);
        // create the mTextures
        for (int i = 0; i < 6; i++)
        {
            createTexture(i);
        }

        // assign a texture to our uniform variables
        glUniform1i(mTexUniform[0], 0);
        checkGlError("glUniform1i 0 initial");
        glUniform1i(mTexUniform[1], 1);
        checkGlError("glUniform1i 1 initial");
        glUniform1i(mTexUniform[2], 2);
        checkGlError("glUniform1i 2 initial");
    }
}

void OGLRenderer::render()
{
    if (mPaused)
    {
        return;
    }

    uint64_t startTime = mud::TimeVal::mono().toMilliSeconds();

    static int textureToggle = 0;

    int frameWidth = mFrame->mWidth;
    int frameHeight = mFrame->mHeight;
    int bufferHeight = frameHeight;
    int bufferWidth = (float)mFrame->mStrides[0];

    //In rare cases a decoder can tell us it decoded a frame without actually returning a buffer
    // this will prevent that bad frame from being rendered
    if (bufferWidth <= 0) {
        return;
    }
    
    if (mNewRightEdgeCoord == 0.0f)
    {
        // Crop edge of texture
        mNewRightEdgeCoord = (float)frameWidth / bufferWidth;
        mTextureCoords[2] = mNewRightEdgeCoord;
        mTextureCoords[6] = mNewRightEdgeCoord;

        /*
         // change to mTextureCoords at these offsets if we
         // re-enable rotation
         gTextureCoordsLandscapeLeft[6] = newRightEdgeCoord;
         gTextureCoordsLandscapeLeft[4] = newRightEdgeCoord;

         gTextureCoordsLandscapeRight[0] = newRightEdgeCoord;
         gTextureCoordsLandscapeRight[2] = newRightEdgeCoord;
         */
    }
    if (mNewBotEdgeCoord == 0.0f)
    {
        // Crop bot edge of texture
        mNewBotEdgeCoord = (float)frameHeight / bufferHeight;
        mTextureCoords[1] = mNewBotEdgeCoord;
        mTextureCoords[3] = mNewBotEdgeCoord;

        /*
         // change to mTextureCoords at these offsets if we
         // re-enable rotation
         gTextureCoordsLandscapeLeft[3] = newBotEdgeCoord;
         gTextureCoordsLandscapeLeft[7] = newBotEdgeCoord;

         gTextureCoordsLandscapeRight[5] = newBotEdgeCoord;
         gTextureCoordsLandscapeRight[1] = newBotEdgeCoord;
         */
    }

    if (mDecodeType==VideoDecoder::DECODE_PLANES444)
    {
        bindAndPopulateTexture(textureToggle, 0, mTexUniform[0], (GLubyte *)mFrame->mPlanes[0], mFrame->mStrides[0], mFrame->mHeight);
        bindAndPopulateTexture(textureToggle, 1, mTexUniform[1], (GLubyte *)mFrame->mPlanes[1], mFrame->mStrides[1], mFrame->mHeight);
        bindAndPopulateTexture(textureToggle, 2, mTexUniform[2], (GLubyte *)mFrame->mPlanes[2], mFrame->mStrides[2], mFrame->mHeight);
    }
    else if (mDecodeType != VideoDecoder::DECODE_PBUFFER) // PLANES or INTERLEAVED
    {
        bindAndPopulateTexture(textureToggle, 0, mTexUniform[0], (GLubyte *)mFrame->mPlanes[0], mFrame->mStrides[0], mFrame->mHeight);
        bindAndPopulateTexture(textureToggle, 1, mTexUniform[1], (GLubyte *)mFrame->mPlanes[1], mFrame->mStrides[1], mFrame->mHeight / 2);
        bindAndPopulateTexture(textureToggle, 2, mTexUniform[2], (GLubyte *)mFrame->mPlanes[2], mFrame->mStrides[2], mFrame->mHeight / 2);
    }
    else // PBUFFER
    {
#if ANDROID
        glUseProgram(mProgram);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_EXTERNAL_OES, mTextures[0]);
        glUniform1i(mTexUniform[0], 0);
        checkGlError("glUniform1i");
#else
        assert(false);
#endif
    }

    if (mFrame)
    {
        textureToggle ^= 1;
    }

    uint64_t finishTime = mud::TimeVal::mono().toMilliSeconds();
    uint32_t totalTime = (uint32_t)(finishTime - startTime);

    // We're trying for 30FPS, so anything greater than 33ms is a problem
    if (totalTime > 32)
    {
        LOGW("renderTime: %dms", totalTime);
    }
    mFrame = NULL;
}

int OGLRenderer::draw()
{
    if (mExiting && mProgram)
    {
        glDeleteProgram(mProgram);
        mProgram = 0;
    }

    if (mPaused)
    {
        return 0;
    }

    if (mDecodeType == VideoDecoder::DECODE_UNDEFINED)
    {
        LOGE("Decoder type undefined!!");
        assert(false);
    }

    // If mActiveDecodeType isn't the current (expected) type,
    // then we need to (re)initialize OpenGL
    if (mDecodeType != mActiveDecodeType)
    {
        glInit();
    }

    glViewport(0, 0, mDisplayWidth, mDisplayHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glViewport(mWidthOffset, mHeightOffset + mKeyboardOffset + mBaseKeyboardOffset, mWidth, mHeight);

    checkGlError("glViewport");
    if (mDecodeType == VideoDecoder::DECODE_UNDEFINED)
    {
        LOGW("Decoder type undefined in draw!!");
        mFrameValid = false;
    }

    // Check to see if the queue has a frame in it, and if so, render it.
    int frameCount = checkQueue();

    // If a frame has been successfully rendered, then draw.
    if (mFrameValid)
    {
        /* draw the surface */
        glVertexAttribPointer(mvPositionHandle, 2, GL_FLOAT, GL_FALSE, 0, gTriangleVertices);
        checkGlError("glVertexAttribPointer");
        glEnableVertexAttribArray(mvPositionHandle);
        checkGlError("glEnableVertexAttribArray");
        glVertexAttribPointer(mvVertexTexCoord, 2, GL_FLOAT, GL_FALSE, 0, mTextureCoords);
        checkGlError("glVertexAttribPointer");
        glEnableVertexAttribArray(mvVertexTexCoord);
        checkGlError("glEnableVertexAttribArray");
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        checkGlError("glDrawArrays");
    }
    else
    {
        // No frame yet; clear.
        glClear(GL_COLOR_BUFFER_BIT);
    }
    return frameCount;
}

void OGLRenderer::requestFrame()
{
    if (mPaused)
    {
        return;
    }
    // Call the platform to request a new OpenGL frame draw
    platformNewFrame();
}

void OGLRenderer::pause(bool pause)
{
    if (pause)
    {
        LOGW("Pause(true)");
        mActiveDecodeType = VideoDecoder::DECODE_UNDEFINED;
        mProgram = 0;
        mvPositionHandle = 0;
        mvVertexTexCoord = 0;
        mNewRightEdgeCoord = 0;
        mNewBotEdgeCoord = 0;
        mPaused = true;
    }
    else
    {
        mPaused = false;
    }
}

void OGLRenderer::setKeyboardOffset(int offset)
{
#if ANDROID // on Android we offset the keyboard
    mKeyboardOffset = offset;
#endif
}

void OGLRenderer::clearScreen()
{
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

/**
 * Provide Chroma sampling capability
 */
bool OGLRenderer::isChromaSamplingSupported(XStxChromaSampling chromaSampling)
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
bool OGLRenderer::receivedClientConfiguration(const XStxClientConfiguration *config)
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

