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


/**
 * @file log.h
 * Logging utility functions.
 */
#ifndef _included_log_h
#define _included_log_h

#ifndef DOXYGEN
#include <android/log.h>
#endif

#ifndef LOG_TAG
/**
 * In each file where you include log.h, you should #undef LOG_TAG and
 * redefine it to specify the file you're in. That way the log tag will
 * match the file you're using.
 */
#define LOG_TAG "StxExampleClient"
#endif

/**
 * "Verbose" level log. Takes a printf-style string plus arbitrary
 * printf-style parameters.
 */
#define  LOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,LOG_TAG,__VA_ARGS__)
/**
 * "Info" level log. Takes a printf-style string plus arbitrary
 * printf-style parameters.
 */
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
/**
 * "Error" level log. Takes a printf-style string plus
 * arbitrary printf-style parameters.
 */
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
/**
 * "Warning" level log. Takes a printf-style string plus
 * arbitrary printf-style parameters.
 */
#define  LOGW(...)  __android_log_print(ANDROID_LOG_WARN,LOG_TAG,__VA_ARGS__)
/**
 * "Debug" level log. Takes a printf-style string plus
 * arbitrary printf-style parameters.
 */
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)
#endif
