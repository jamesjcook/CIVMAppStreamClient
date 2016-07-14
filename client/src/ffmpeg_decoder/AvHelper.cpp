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



#include <assert.h>
#include <cstdarg>
#include <cstdio>

extern "C"
{
// FFmpeg libraries
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

#include "MUD/threading/ScopeLock.h"
#include "MUD/base/TimeVal.h"
#include "AvHelper.h"

#undef LOG_TAG
#define LOG_TAG "AvHelper"
#include "log.h"

using namespace mud;

// for thread-safety
static mud::SimpleLock gLock;
static int gRefCount = 0;
static mud::TimeVal gLastLog;
static int gSuppressedCount = 0;

/**
 * A class to coordinate working with FFmpeg
 *   @ingroup Examples
 *   @{
 */
void av_log_callback(void *, int level, const char *format, va_list args)
{

#if 1 // suppress AVHelper spam

    gSuppressedCount++;

    if (gLastLog.elapsedMono().toSeconds() > 5)
    {
        LOGW( "%d (total) messages suppressed\n", gSuppressedCount);
        gLastLog.resetMono();
    }

#else

    char buf[1024];

    vsnprintf(buf, sizeof(buf), format, args);

    switch (level)
    {
    case AV_LOG_QUIET:
    case AV_LOG_PANIC:
    case AV_LOG_FATAL:
    case AV_LOG_ERROR:
        LOGE("Error: %s\n",  buf);
        break;

    case AV_LOG_WARNING:
        LOGW("Warning: %s\n",  buf);
        break;

    case AV_LOG_INFO:
        LOGI("Info: %s\n",  buf);
        break;

    case AV_LOG_VERBOSE:
        LOGV("Verbose: %s\n",  buf);
        break;

    case AV_LOG_DEBUG:
        LOGD("Debug: %s\n",  buf);
        break;

    default:
        LOGE("Unknown AV log level: %d for message \"%s\"\n",  level, buf);
        break;
    }

#endif

}

void AvHelper::initialize()
{

    mud::ScopeLock scopeLock(gLock);

    if (0 == gRefCount)
    {

        gLastLog.resetMono();
        gSuppressedCount = 0;

        av_log_set_callback(&av_log_callback);

        avcodec_register_all();
        av_register_all();

    }

    gRefCount++;
}

void AvHelper::terminate()
{

    mud::ScopeLock scopeLock(gLock);

    gRefCount--;

    if (0 == gRefCount)
    {
        av_log_set_callback(&av_log_default_callback);
    }
}

