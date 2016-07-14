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


#ifndef _included_log_h
#define _included_log_h

#ifndef LOG_TAG
#define LOG_TAG "AppstreamSampleClient"
#endif

#define  LOGV(...)  {printf("%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n");}
#define  LOGI(...)  {printf("%s: ", LOG_TAG); printf(__VA_ARGS__); printf("\n");}
#define  LOGE(...)  {fprintf(stderr, "%s: ", LOG_TAG); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define  LOGW(...)  {fprintf(stderr, "%s: ", LOG_TAG); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#define  LOGD(...)  {fprintf(stderr, "%s: ", LOG_TAG); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n");}
#endif
