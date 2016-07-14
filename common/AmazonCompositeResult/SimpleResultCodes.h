/** 
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * 
 * Licensed under the Amazon Software License (the "License"). You may not
 * use this file except in compliance with the License. A copy of the License
 *  is located at
 * 
 *       http://aws.amazon.com/asl/  
 *        
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR 
 * CONDITIONS OF ANY KIND, express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 */

#ifndef __SIMPLE_RESULT_CODES_
#define __SIMPLE_RESULT_CODES_

#include <stdint.h>

/**
 * +====================================================================+
 * |         Domain                  |           Code Ranges            |
 * +=================================+==================================+
 * | Generic                         | 0 - 799                          |
 * +---------------------------------+----------------------------------+
 * | Threading                       | 800 - 999                        |
 * +---------------------------------+----------------------------------+
 * | Network                         | 1000 - 1999                      |
 * +---------------------------------+----------------------------------+
 * | TLS                             | 2000 - 2999                      |
 * +---------------------------------+----------------------------------+
 * | Windows specific                | 0xFF000000 - 0xFF00FFFF          |
 * +---------------------------------+----------------------------------+
*/

namespace acr
{

///Generic result codes.
#define SIMPLE_RESULT_OK                    0
#define SIMPLE_RESULT_UNKNOWN               (SIMPLE_RESULT_OK + 1)
#define SIMPLE_RESULT_INVALID               (SIMPLE_RESULT_UNKNOWN + 1)
#define SIMPLE_RESULT_FAULT                 (SIMPLE_RESULT_INVALID + 1)
#define SIMPLE_RESULT_PERMISSION_ERROR      (SIMPLE_RESULT_FAULT + 1)
#define SIMPLE_RESULT_NO_MEMORY             (SIMPLE_RESULT_PERMISSION_ERROR + 1)
#define SIMPLE_RESULT_BAD_FD                (SIMPLE_RESULT_NO_MEMORY + 1)
#define SIMPLE_RESULT_NO_FD                 (SIMPLE_RESULT_BAD_FD + 1) 
#define SIMPLE_RESULT_OUT_OF_BOUND          (SIMPLE_RESULT_NO_FD + 1)
#define SIMPLE_RESULT_TIMEOUT               (SIMPLE_RESULT_OUT_OF_BOUND + 1)
#define SIMPLE_RESULT_AGAIN                 (SIMPLE_RESULT_TIMEOUT + 1)
#define SIMPLE_RESULT_BUSY                  (SIMPLE_RESULT_AGAIN + 1)
#define SIMPLE_RESULT_ABORTED               (SIMPLE_RESULT_BUSY + 1)
#define SIMPLE_RESULT_INTERRUPTED           (SIMPLE_RESULT_ABORTED + 1)
#define SIMPLE_RESULT_NOT_FOUND             (SIMPLE_RESULT_INTERRUPTED + 1)
#define SIMPLE_RESULT_DUPLICATED            (SIMPLE_RESULT_NOT_FOUND + 1)
#define SIMPLE_RESULT_NOT_SUPPORTED         (SIMPLE_RESULT_DUPLICATED + 1)
#define SIMPLE_RESULT_IN_PROGRESS           (SIMPLE_RESULT_NOT_SUPPORTED + 1)
#define SIMPLE_RESULT_ALREADY               (SIMPLE_RESULT_IN_PROGRESS + 1)
#define SIMPLE_RESULT_PENDING               (SIMPLE_RESULT_ALREADY + 1)
#define SIMPLE_RESULT_DEAD_LOCK             (SIMPLE_RESULT_PENDING + 1)
#define SIMPLE_RESULT_NOT_INITIALIZED       (SIMPLE_RESULT_DEAD_LOCK + 1)
#define SIMPLE_RESULT_ALREADY_INITIALIZED   (SIMPLE_RESULT_NOT_INITIALIZED + 1)
#define SIMPLE_RESULT_INITIALIZATION_FAILED (SIMPLE_RESULT_ALREADY_INITIALIZED + 1)
#define SIMPLE_RESULT_NOT_DEINITIALIZED     (SIMPLE_RESULT_INITIALIZATION_FAILED + 1)
#define SIMPLE_RESULT_ALREADY_DEINITIALIZED (SIMPLE_RESULT_NOT_DEINITIALIZED + 1)
#define SIMPLE_RESULT_NOT_STARTED           (SIMPLE_RESULT_ALREADY_DEINITIALIZED + 1)
#define SIMPLE_RESULT_ALREADY_STARTED       (SIMPLE_RESULT_NOT_STARTED + 1)
#define SIMPLE_RESULT_NOT_STOPPED           (SIMPLE_RESULT_ALREADY_STARTED + 1)
#define SIMPLE_RESULT_ALREADY_STOPPED       (SIMPLE_RESULT_NOT_STOPPED + 1)
#define SIMPLE_RESULT_NOT_RUNNING           (SIMPLE_RESULT_ALREADY_STOPPED + 1)
#define SIMPLE_RESULT_ALREADY_RUNNING       (SIMPLE_RESULT_NOT_RUNNING + 1)
#define SIMPLE_RESULT_BAD_FILE              (SIMPLE_RESULT_ALREADY_RUNNING + 1)
#define SIMPLE_RESULT_BAD_DIR               (SIMPLE_RESULT_BAD_FILE + 1)
#define SIMPLE_RESULT_MSG_SIZE_ERROR        (SIMPLE_RESULT_BAD_DIR + 1)

///Threading result codes.
#define SIMPLE_RESULT_THREAD_UNKNOWN        800

///Network result codes.
#define SIMPLE_RESULT_NET_UNKNOWN                       1000
#define SIMPLE_RESULT_NET_AF_NOT_SUPPORTED              (SIMPLE_RESULT_NET_UNKNOWN + 1)
#define SIMPLE_RESULT_NET_PROTO_NOT_SUPPORTED           (SIMPLE_RESULT_NET_AF_NOT_SUPPORTED + 1)
#define SIMPLE_RESULT_NET_NOT_SOCKET                    (SIMPLE_RESULT_NET_PROTO_NOT_SUPPORTED + 1)
#define SIMPLE_RESULT_NET_ADDR_IN_USE                   (SIMPLE_RESULT_NET_NOT_SOCKET + 1)
#define SIMPLE_RESULT_NET_ADDR_NOT_AVAILABLE            (SIMPLE_RESULT_NET_ADDR_IN_USE + 1)
#define SIMPLE_RESULT_NET_CONN_ABORTED                  (SIMPLE_RESULT_NET_ADDR_NOT_AVAILABLE + 1)
#define SIMPLE_RESULT_NET_PROTO_ERROR                   (SIMPLE_RESULT_NET_CONN_ABORTED + 1)
#define SIMPLE_RESULT_NET_SOCKET_TYPE_NOT_SUPPORTED     (SIMPLE_RESULT_NET_PROTO_ERROR + 1)
#define SIMPLE_RESULT_NET_CONN_REFUSED                  (SIMPLE_RESULT_NET_SOCKET_TYPE_NOT_SUPPORTED + 1)
#define SIMPLE_RESULT_NET_CONN_RESET                    (SIMPLE_RESULT_NET_CONN_REFUSED + 1)
#define SIMPLE_RESULT_NET_SOCKET_ALREADY_CONNECTED      (SIMPLE_RESULT_NET_CONN_RESET + 1)
#define SIMPLE_RESULT_NET_SOCKET_NOT_CONNECTED          (SIMPLE_RESULT_NET_SOCKET_ALREADY_CONNECTED + 1)
#define SIMPLE_RESULT_NET_NETWORK_UNREACHABLE           (SIMPLE_RESULT_NET_SOCKET_NOT_CONNECTED + 1)
#define SIMPLE_RESULT_NET_HOST_UNREACHABLE              (SIMPLE_RESULT_NET_NETWORK_UNREACHABLE + 1)
#define SIMPLE_RESULT_NET_DEST_ADDR_REQUIRED            (SIMPLE_RESULT_NET_HOST_UNREACHABLE + 1)
#define SIMPLE_RESULT_NET_NETWORK_RESET                 (SIMPLE_RESULT_NET_DEST_ADDR_REQUIRED + 1)
#define SIMPLE_RESULT_NET_NETWORK_DOWN                  (SIMPLE_RESULT_NET_NETWORK_RESET + 1)
#define SIMPLE_RESULT_NET_PROTO_TYPE_WRONG              (SIMPLE_RESULT_NET_NETWORK_DOWN + 1)
#define SIMPLE_RESULT_NET_SOCKET_SHUTDOWN               (SIMPLE_RESULT_NET_PROTO_TYPE_WRONG + 1)

///TLS result codes.
#define SIMPLE_RESULT_TLS_ERROR             2000

///Windows system codes.
#define SIMPLE_RESULT_WINDOWS_SYSTEM_CODE_START (0xFF000000)
#define SIMPLE_RESULT_WINDOWS_SYSTEM_CODE_END   (0xFF00FFFF)

/** Get the string representation of a code. */
const char* getSimpleResultCodeName( uint32_t code );

}

#endif //__SIMPLE_RESULT_CODES_
