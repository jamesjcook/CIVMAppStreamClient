LOCAL_PATH := $(call my-dir)

ifeq "$(STX_SDK_DIR)" ""
    ROOT_PATH := $(LOCAL_PATH)/../../../../..
else
    ROOT_PATH := $(STX_SDK_DIR)
endif

THIRD_PARTY := $(realpath $(ROOT_PATH))/3rdparty/android
FFMPEG := $(THIRD_PARTY)/ffmpeg
OPUS := $(THIRD_PARTY)/opus

CLIENT_PATH := $(LOCAL_PATH)/../../..

include $(CLEAR_VARS)

LOCAL_CPP_FEATURES := rtti exceptions

LOCAL_MODULE    := appstreamsample
LOCAL_SRC_FILES := \
    $(CLIENT_PATH)/src/AudioModule.cpp \
    $(CLIENT_PATH)/src/AudioRenderer.cpp \
    $(CLIENT_PATH)/src/ffmpeg_decoder/H264ToYuv.cpp \
    $(CLIENT_PATH)/src/ffmpeg_decoder/AVHelper.cpp \
    $(CLIENT_PATH)/src/VideoModule.cpp \
    $(CLIENT_PATH)/src/VideoRenderer.cpp \
    $(CLIENT_PATH)/src/AppStreamWrapper.cpp \
    $(CLIENT_PATH)/src/opus_decoder/OpusDecoder.cpp \
    AudioPipeline.cpp \
    AndroidAudioRenderer.cpp \
    AndroidVideoDecoder.cpp \
    AndroidOpusDecoder.cpp \
    OpenSLAudio.cpp \

ifeq ($(UNITY),1)
    LOCAL_SRC_FILES+=$(CLIENT_PATH)/src/unity/android/UnityAppStreamWrapper.cpp
    LOCAL_SRC_FILES+=$(CLIENT_PATH)/src/unity/opengl_renderer/OGLUnityRenderer.cpp
    LOCAL_SRC_FILES+=$(CLIENT_PATH)/src/unity/android/VideoPipeline.cpp
    LOCAL_SRC_FILES+=$(CLIENT_PATH)/src/unity/android/jniBindings.cpp
else
    LOCAL_SRC_FILES+=$(CLIENT_PATH)/src/opengl_renderer/OGLRenderer.cpp
    LOCAL_SRC_FILES+=VideoPipeline.cpp
    LOCAL_SRC_FILES+=jniBindings.cpp
endif

LOCAL_ARM_MODE := arm

LOCAL_CFLAGS += -D__STDINT_MACROS
LOCAL_LDLIBS := -lz -llog -lGLESv2 -lOpenSLES

LOCAL_C_INCLUDES := $(ROOT_PATH)/3rdparty $(ROOT_PATH)/example_src/common $(ROOT_PATH)/include $(FFMPEG)/include $(OPUS)/include $(CLIENT_PATH)/src

LOCAL_STATIC_LIBRARIES := opus
LOCAL_SHARED_LIBRARIES := XStxClientLibraryShared avutil avformat avcodec swresample

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := opus
LOCAL_SRC_FILES := $(OPUS)/lib/$(TARGET_ARCH_ABI)/libopus.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := XStxClientLibraryShared
LOCAL_SRC_FILES := $(ROOT_PATH)/lib/android/$(TARGET_ARCH_ABI)/libXStxClientLibraryShared.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avcodec
LOCAL_SRC_FILES := $(FFMPEG)/lib/$(TARGET_ARCH_ABI)/libavcodec.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avutil
LOCAL_SRC_FILES := $(FFMPEG)/lib/$(TARGET_ARCH_ABI)/libavutil.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := avformat
LOCAL_SRC_FILES := $(FFMPEG)/lib/$(TARGET_ARCH_ABI)/libavformat.so
include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := swresample
LOCAL_SRC_FILES := $(FFMPEG)/lib/$(TARGET_ARCH_ABI)/libswresample.so
include $(PREBUILT_SHARED_LIBRARY)
