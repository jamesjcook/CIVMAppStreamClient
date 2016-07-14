/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may not use
 * this file except in compliance with the License. A copy of the License is
 * located at
 *
 *      http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 * CONDITIONS OF ANY KIND, express or implied. See the License for the
 * specific language governing permissions and limitations under the License.
 */


#include "platformBindings.h"
#include "jniBindings.h"

#include <jni.h>

#undef LOG_TAG
#define LOG_TAG "jniBindings"
#include "log.h"
#include "AppStreamWrapper.h"
#include "OpenSLAudio.h"

#include <sys/atomics.h>

AppStreamWrapper *gAppStreamWrapper = NULL;
pthread_key_t gCurrentThreadJNI(0);


int initLibrary(JNIEnv *, jobject)
{
    if (gAppStreamWrapper == NULL)
    {
        gAppStreamWrapper = new AppStreamWrapper();
        return gAppStreamWrapper->init();
    }

    return -1;
}

bool androidCommitFrame();

void step(JNIEnv *env, jobject objectOrClass)
{
    androidCommitFrame();
    assert(gAppStreamWrapper);
    gAppStreamWrapper->step();
}
void stop(JNIEnv *env, jobject objectOrClass)
{
    LOGV("gAppStreamWrapper->stop");
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->stop();
        delete gAppStreamWrapper;
        gAppStreamWrapper = NULL;
    }
}

void connect(JNIEnv *env, jobject objectOrClass, jstring address)
{
    if (!gAppStreamWrapper)
    {
        initLibrary(NULL,NULL);
    }
    assert(gAppStreamWrapper);
    const char *utf8 = env->GetStringUTFChars(address, NULL);
    gAppStreamWrapper->connect(utf8);
    env->ReleaseStringUTFChars(address, utf8);
}

void _setupGraphics(JNIEnv *env, jobject objectOrClass, int w, int h)
{
    gAppStreamWrapper->initGraphics(w, h, false);
}

void setKeyboardOffset(JNIEnv *env, jobject objectOrClass, int offset)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->setKeyboardOffset(offset);
    }
}

void keyPress(JNIEnv *env, jobject objectOrClass, int key, jboolean down)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->keyPress(key, down != 0);
    }
}

void mouseEvent(JNIEnv *env, jobject objectOrClass, int x, int y, int flags)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->mouseEvent(x, y, flags);
    }
}

void jniPause(JNIEnv *env, jobject objectOrClass, jboolean pause)
{
    if (gAppStreamWrapper)
    {
        gAppStreamWrapper->pause(pause != 0);
    }
}

bool gHardwareDecoderEnabled = true;
jobject gHardwareDecoder = NULL;
jmethodID gGetNextBuffer = NULL;
jmethodID gCommitBuffer = NULL;
jmethodID gGetOutputBuffer = NULL;
jmethodID gReleaseOutputBuffer = NULL;
jmethodID gCloseHardwareDecoder = NULL;
jmethodID gGetOutputBufferWidth = NULL;
jmethodID gGetOutputBufferHeight = NULL;
jmethodID gGetOutputBufferSliceHeight = NULL;
jmethodID gGetOutputBufferIsPlanar = NULL;
jmethodID gGetOutputBufferIsValid = NULL;
jmethodID gInitDecoderFromGL = NULL;
jmethodID gCommitFrame = NULL;
jmethodID gGetLevel = NULL;
jmethodID gGetProfile = NULL;
jmethodID gOnReconnect = NULL;

void setHardwareDecoder(JNIEnv *env, jobject objectOrClass, jobject hardwareDecoder)
{
    if (hardwareDecoder == NULL)
    {
        gHardwareDecoderEnabled = false;
        return;
    }
    if (gHardwareDecoder)
    {
        gHardwareDecoderEnabled = true;
        return;
    }

    jobject localHardwareDecoder = env->NewGlobalRef(hardwareDecoder);

    jclass HardwareDecoder = env->GetObjectClass(localHardwareDecoder);

    gGetNextBuffer = env->GetMethodID(HardwareDecoder, "getNextBuffer", "()Ljava/nio/ByteBuffer;");
    gCommitBuffer = env->GetMethodID(HardwareDecoder, "commitBuffer", "(II)V");
    gCloseHardwareDecoder = env->GetMethodID(HardwareDecoder, "close", "()V");

    gGetOutputBuffer = env->GetMethodID(HardwareDecoder, "getOutputBuffer", "()Ljava/nio/ByteBuffer;");
    gReleaseOutputBuffer = env->GetMethodID(HardwareDecoder, "releaseOutputBuffer", "()V");

    gGetLevel = env->GetMethodID(HardwareDecoder, "getLevel", "()I");
    gGetProfile = env->GetMethodID(HardwareDecoder, "getProfile", "()I");

    gGetOutputBufferWidth = env->GetMethodID(HardwareDecoder, "getOutputBufferWidth", "()I");
    gGetOutputBufferHeight = env->GetMethodID(HardwareDecoder, "getOutputBufferHeight", "()I");
    gGetOutputBufferSliceHeight = env->GetMethodID(HardwareDecoder, "getOutputBufferSliceHeight", "()I");
    gGetOutputBufferIsPlanar = env->GetMethodID(HardwareDecoder, "getOutputBufferIsPlanar", "()Z");
    gCommitFrame = env->GetMethodID(HardwareDecoder, "commitFrame", "()Z");
    gGetOutputBufferIsValid = env->GetMethodID(HardwareDecoder, "getOutputBufferIsValid", "()Z");
    gInitDecoderFromGL = env->GetMethodID(HardwareDecoder, "getGLTexture", "(I)I");

    // Don't set it until we're completely initialized, or there's a danger
    // that another thread will try to use the hardware decoder before it's
    // initialized.
    __atomic_swap((int)localHardwareDecoder, (volatile int *)&gHardwareDecoder);
}

JavaVM *gVM = NULL;

JNIEnv* getOrAttachEnv()
{
    JNIEnv *env = NULL;

    if ((env = (JNIEnv *)pthread_getspecific(gCurrentThreadJNI)) == NULL)
    {
        gVM->GetEnv((void **)&env, JNI_VERSION_1_6);
        if (!env)
        {
            gVM->AttachCurrentThread(&env, NULL);
        }
        pthread_setspecific(gCurrentThreadJNI, env);
    }

    return env;
}

static void *sHWBuffer = NULL;
static void *sHWOutBuffer = NULL;

bool androidHWDecodeAvailable()
{
    return gHardwareDecoderEnabled && (gHardwareDecoder != NULL);
}

void* androidGetHWBuffer(int *size)
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetHWBuffer with null gHardwareDecoder");
        return NULL;
    }
    JNIEnv *env = getOrAttachEnv();
    jobject tempbuffer = env->CallObjectMethod(gHardwareDecoder, gGetNextBuffer);

    if (!tempbuffer)
    {
        LOGE("Bad state in androidGetHWBuffer(): No Buffer available.");
        return NULL;
    }

    sHWBuffer = env->GetDirectBufferAddress(tempbuffer);
    *size = env->GetDirectBufferCapacity(tempbuffer);

    env->DeleteLocalRef(tempbuffer);

    return sHWBuffer;
}

void androidReleaseHWBuffer(int size, int time)
{
    JNIEnv *env = getOrAttachEnv();
    sHWBuffer = NULL;

    env->CallVoidMethod(gHardwareDecoder, gCommitBuffer, size, time);
}

void* androidGetOutputBuffer(int *size)
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return NULL;
    }

    JNIEnv *env = getOrAttachEnv();
    jobject tempbuffer = env->CallObjectMethod(gHardwareDecoder, gGetOutputBuffer);

    if (!tempbuffer)
    {
        return NULL;
    }

    *size = env->GetDirectBufferCapacity(tempbuffer);
    sHWOutBuffer = env->GetDirectBufferAddress(tempbuffer);

    env->DeleteLocalRef(tempbuffer);
    return sHWOutBuffer;
}

int androidGetOutputBufferWidth()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return 0;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetOutputBufferWidth);
}

int androidGetProfile()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return 0;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetProfile);
}

int androidGetLevel()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return 0;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetLevel);
}

int androidGetOutputBufferSliceHeight()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return 0;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetOutputBufferSliceHeight);
}

int androidGetOutputBufferHeight()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return 0;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetOutputBufferHeight);
}

bool androidGetOutputBufferIsPlanar()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return NULL;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetOutputBufferIsPlanar);
}

bool androidCommitFrame()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return false;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallBooleanMethod(gHardwareDecoder, gCommitFrame) != 0;
}

bool androidGetOutputBufferIsValid()
{
    if (!gHardwareDecoder)
    {
        LOGE("Call to androidGetOutputBuffer with null gHardwareDecoder");
        return NULL;
    }

    JNIEnv *env = getOrAttachEnv();
    return env->CallIntMethod(gHardwareDecoder, gGetOutputBufferIsValid);
}

void androidReleaseOutputBuffer()
{
    JNIEnv *env = getOrAttachEnv();
    sHWOutBuffer = NULL;

    env->CallVoidMethod(gHardwareDecoder, gReleaseOutputBuffer);
}

void joystickState(JNIEnv *env, jobject objectOrClass,
                   jint buttons,
                   jint leftTrigger,
                   jint rightTrigger,
                   jint thumbLX,
                   jint thumbLY,
                   jint thumbRX,
                   jint thumbRY)
{
    gAppStreamWrapper->joystickState(buttons, leftTrigger >> 8, rightTrigger >> 8, thumbLX, thumbLY, thumbRX, thumbRY);
}

// Function signature should be:
//ReturnType (*fnPtr)(JNIEnv *env, jobject objectOrClass, ...);

JNINativeMethod method[] = {
//		"JavaFnName", "Parameters", "Pointer to function"
    { "initLibrary", "()I", (void *)initLibrary },
    { "setupGraphics", "(II)V", (void *)_setupGraphics },
    { "audioSetPlaying", "(Z)V", (void *)OpenSLSetPlaying },
    { "keyPress", "(IZ)V", (void *)keyPress },
    { "setKeyboardOffset", "(I)V", (void *)setKeyboardOffset },
    { "connect", "(Ljava/lang/String;)V", (void *)connect },
    { "step", "()V", (void *)step },
    { "stop", "()V", (void *)stop },
    { "mouseEvent", "(III)V", (void *)mouseEvent },
    { "pause", "(Z)V", (void *)jniPause },
    { "setHardwareDecoder", "(Lcom/amazon/appstream/HardwareDecoder;)V", (void *)setHardwareDecoder },
    { "joystickState", "(IIIIIII)V", (void *)joystickState },
};

jclass gAppStreamInterface = NULL;
jmethodID gNewFrame = NULL;
jmethodID gConnectSuccess = NULL;
jmethodID gErrorMessage = NULL;

void _platformDetachThread(void *env)
{
    if (env)
    {
        gVM->DetachCurrentThread();
    }
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
    gVM = vm;

    pthread_key_create(&gCurrentThreadJNI, _platformDetachThread);

    JNIEnv *env = getOrAttachEnv();

    // Get jclass with env->FindClass.
    jclass asi = env->FindClass("com/amazon/appstream/AppStreamInterface");

    gAppStreamInterface = (jclass)env->NewGlobalRef(asi);
    gNewFrame = env->GetStaticMethodID(gAppStreamInterface, "newFrame", "()V");
    gConnectSuccess = env->GetStaticMethodID(gAppStreamInterface, "onConnectSuccess", "()V");
    gErrorMessage = env->GetStaticMethodID(gAppStreamInterface, "errorMessage", "(ZLjava/lang/String;)V");
    gOnReconnect = env->GetStaticMethodID(gAppStreamInterface, "onReconnecting", "(Ljava/lang/String;)V");

    // Register methods with env->RegisterNatives.
    env->RegisterNatives(asi, method, sizeof(method) / sizeof(JNINativeMethod));

    return JNI_VERSION_1_6;
}

void platformOnConnectSuccess()
{
    JNIEnv *env = getOrAttachEnv();
    env->CallStaticVoidMethod(gAppStreamInterface, gConnectSuccess);
}

void platformNewFrame()
{
    JNIEnv *env = getOrAttachEnv();
    env->CallStaticVoidMethod(gAppStreamInterface, gNewFrame);
}

void platformErrorMessage(bool fatal, const char *message)
{
    JNIEnv *env = getOrAttachEnv();
    jstring jMessage = env->NewStringUTF(message);
    env->CallStaticVoidMethod(gAppStreamInterface, gErrorMessage, (jboolean)fatal, jMessage);
}

int platformBindVideoTexture(int textureID)
{
    JNIEnv *env = getOrAttachEnv();
    sHWBuffer = NULL;

    return env->CallIntMethod(gHardwareDecoder, gInitDecoderFromGL, textureID);
}

void platformOnReconnecting(uint32_t timeoutMs, const char *message)
{
    JNIEnv *env = getOrAttachEnv();

    jstring jMessage = env->NewStringUTF(message);

    env->CallStaticVoidMethod(gAppStreamInterface, gOnReconnect, jMessage);
}

void platformOnReconnected()
{
    JNIEnv *env = getOrAttachEnv();
    env->CallStaticVoidMethod(gAppStreamInterface, gOnReconnect, NULL);
}

