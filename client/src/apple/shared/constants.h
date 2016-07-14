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


#ifndef AppStreamSampleClient_constants_h
#define AppStreamSampleClient_constants_h

#define kXSTX_CLIENT_STOPPED_NOTIFICATION       @"XSTX_CLIENT_STOPPED"
#define kXSTX_CLIENT_READY_NOTIFICATION         @"XSTX_CLIENT_READY"
#define kXSTX_VIDEO_SIZE_CHANGED_NOTIFICATION   @"XSTX_VIDEO_SIZE_CHANGED"
#define kXSTX_RECONNECTING_NOTIFICATION         @"XSTX_RECONNECTING_NOTIFICATION"
#define kXSTX_RECONNECTED_NOTIFICATION          @"XSTX_RECONNECTED_NOTIFICATION"
#define kXSTX_CLIENT_STOP_MESSAGE               @"XSTX_STOP_MESSAGE"
#define kXSTX_CLIENT_STOP_IS_FATAL              @"XSTX_STOP_FATAL"
#define kXSTX_RECONNECTING_MESSAGE              @"XSTX_RECONNECTING_MESSAGE"
#define kXSTX_RECONNECTING_TIMEOUT_IN_MS        @"XSTX_RECONNECTING_TIMEOUT_IN_MS"

#define kXSTX_USER_STOP_NOTIFICATION            @"XSTX_USER_STOP_NOTIFICATION"


// mouse emulation flags  - see http://msdn.microsoft.com/en-us/library/windows/desktop/ms645578(v=vs.85).aspx
static const uint32_t RI_MOUSE_LEFT_BUTTON_DOWN     = 0x0001;
static const uint32_t RI_MOUSE_LEFT_BUTTON_UP       = 0x0002;
static const uint32_t RI_MOUSE_MIDDLE_BUTTON_DOWN   = 0x0010;
static const uint32_t RI_MOUSE_MIDDLE_BUTTON_UP     = 0x0020;
static const uint32_t RI_MOUSE_RIGHT_BUTTON_DOWN    = 0x0004;
static const uint32_t RI_MOUSE_RIGHT_BUTTON_UP      = 0x0008;
static const uint32_t RI_MOUSE_BUTTON_1_DOWN        = 0x0001;
static const uint32_t RI_MOUSE_BUTTON_1_UP          = 0x0002;
static const uint32_t RI_MOUSE_BUTTON_2_DOWN        = 0x0004;
static const uint32_t RI_MOUSE_BUTTON_2_UP          = 0x0008;
static const uint32_t RI_MOUSE_BUTTON_3_DOWN        = 0x0010;
static const uint32_t RI_MOUSE_BUTTON_3_UP          = 0x0020;

// a subset of some common windowsvirtual keys
// http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx
static const uint32_t kBACK                         = 0x08;
static const uint32_t kTAB                          = 0x09;
static const uint32_t kENTER                        = 0x0D;
static const uint32_t kSHIFT                        = 0x10;
static const uint32_t kCONTROL                      = 0x11;
static const uint32_t kMENU                         = 0x12; //alt
static const uint32_t kESCAPE                       = 0x1B;
static const uint32_t kCOMMA                        = 0xBC;
static const uint32_t kMINUS                        = 0xBD;
static const uint32_t kPERIOD                       = 0xBE;
static const uint32_t kFSLASH                       = 0xBF;
static const uint32_t kSEMICOLON                    = 0xBA;
static const uint32_t kUNDERSCORE                   = 0xBD;
static const uint32_t kTILDE                        = 0xBD;



#define PLACEHOLDER_DES_ADDRESS         @"des.example.com"
#define PLACEHOLDER_DES_APPID           @"a1b2c3d4-e5f6-a7b8-c9d0-e1f2a3b4c5d6"
#define PLACEHOLDER_DES_USER            @"test@example.com"
#define PLACEHOLDER_STANDALONE_ADDRESS  @"123.123.123.123"

#define ERROR_MISSING_SERVER_ADDRESS    @"Server address is required"
#define ERROR_INVALID_SERVER_ADDRESS    @"Server address is invalid"
#define ERROR_MISSING_APP_ID            @"Application ID is required"
#define ERROR_MISSING_USER_ID           @"User ID is required"



#endif
