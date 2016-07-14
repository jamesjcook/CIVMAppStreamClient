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

#ifndef _MUD_UNIX_UTIL_H_
#define _MUD_UNIX_UTIL_H_

#include <stdint.h>
#include <errno.h>

#include "AmazonCompositeResult/SimpleResultCodes.h"

namespace mud
{

#define ERRNO_TO_SIMPLE_RESULT_CODE( errorNumber, simpleResultCode ) case (errorNumber): {return simpleResultCode;}

/**
 *  Unix utility class.
 */
class UnixUtil
{
public:
    /** Convert the errno to Simple Result Code. */
    static uint32_t errnoToSimpleResultCode( int errorNumber )
    {
        uint32_t retVal = SIMPLE_RESULT_UNKNOWN;

        switch (errorNumber)
        {
            ERRNO_TO_SIMPLE_RESULT_CODE( EAGAIN, SIMPLE_RESULT_AGAIN )
            //EAGAIN has the same value with EWOULDBLOCK, so the following is not necessary.
            //ERRNO_TO_SIMPLE_RESULT_CODE( EWOULDBLOCK, SIMPLE_RESULT_AGAIN )
            ERRNO_TO_SIMPLE_RESULT_CODE( EINVAL, SIMPLE_RESULT_INVALID )
            ERRNO_TO_SIMPLE_RESULT_CODE( EPERM, SIMPLE_RESULT_PERMISSION_ERROR )
            ERRNO_TO_SIMPLE_RESULT_CODE( EACCES, SIMPLE_RESULT_PERMISSION_ERROR )
            ERRNO_TO_SIMPLE_RESULT_CODE( ESRCH, SIMPLE_RESULT_NOT_FOUND )
            ERRNO_TO_SIMPLE_RESULT_CODE( EDEADLK, SIMPLE_RESULT_DEAD_LOCK )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENOBUFS, SIMPLE_RESULT_NO_MEMORY )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENOMEM, SIMPLE_RESULT_NO_MEMORY )
            ERRNO_TO_SIMPLE_RESULT_CODE( EBADF, SIMPLE_RESULT_BAD_FD )
            ERRNO_TO_SIMPLE_RESULT_CODE( EFAULT, SIMPLE_RESULT_FAULT )
            ERRNO_TO_SIMPLE_RESULT_CODE( EMFILE, SIMPLE_RESULT_NO_FD )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENFILE, SIMPLE_RESULT_NO_FD )
            ERRNO_TO_SIMPLE_RESULT_CODE( EINTR, SIMPLE_RESULT_INTERRUPTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( EOPNOTSUPP, SIMPLE_RESULT_NOT_SUPPORTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ETIMEDOUT, SIMPLE_RESULT_TIMEOUT )
            ERRNO_TO_SIMPLE_RESULT_CODE( EALREADY, SIMPLE_RESULT_ALREADY )
            ERRNO_TO_SIMPLE_RESULT_CODE( EINPROGRESS, SIMPLE_RESULT_IN_PROGRESS )
            ERRNO_TO_SIMPLE_RESULT_CODE( EMSGSIZE, SIMPLE_RESULT_MSG_SIZE_ERROR )

            ERRNO_TO_SIMPLE_RESULT_CODE( EAFNOSUPPORT, SIMPLE_RESULT_NET_AF_NOT_SUPPORTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( EPROTONOSUPPORT, SIMPLE_RESULT_NET_PROTO_NOT_SUPPORTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENOTSOCK, SIMPLE_RESULT_NET_NOT_SOCKET )
            ERRNO_TO_SIMPLE_RESULT_CODE( EADDRINUSE, SIMPLE_RESULT_NET_ADDR_IN_USE )
            ERRNO_TO_SIMPLE_RESULT_CODE( EADDRNOTAVAIL, SIMPLE_RESULT_NET_ADDR_NOT_AVAILABLE )
            ERRNO_TO_SIMPLE_RESULT_CODE( ECONNABORTED, SIMPLE_RESULT_NET_CONN_ABORTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( EPROTO, SIMPLE_RESULT_NET_PROTO_ERROR )
            ERRNO_TO_SIMPLE_RESULT_CODE( ESOCKTNOSUPPORT, SIMPLE_RESULT_NET_SOCKET_TYPE_NOT_SUPPORTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ECONNREFUSED, SIMPLE_RESULT_NET_CONN_REFUSED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ECONNRESET, SIMPLE_RESULT_NET_CONN_RESET )
            ERRNO_TO_SIMPLE_RESULT_CODE( EISCONN, SIMPLE_RESULT_NET_SOCKET_ALREADY_CONNECTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( EDESTADDRREQ, SIMPLE_RESULT_NET_DEST_ADDR_REQUIRED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENOTCONN, SIMPLE_RESULT_NET_SOCKET_NOT_CONNECTED )
            ERRNO_TO_SIMPLE_RESULT_CODE( ENETUNREACH, SIMPLE_RESULT_NET_NETWORK_UNREACHABLE )

            default:
                retVal = SIMPLE_RESULT_UNKNOWN;
                break;
        }

        return retVal;
    }

    /** Convert the network error to Simple Result Code. */
    static uint32_t netErrToSimpleResultCode( int errorNumber )
    {
        uint32_t retVal = errnoToSimpleResultCode( errorNumber );

        //Capture the network unknown errors.
        if (retVal == SIMPLE_RESULT_UNKNOWN)
        {
            retVal = SIMPLE_RESULT_NET_UNKNOWN;
        }

        return retVal;
    }
};

} //namespace mud

#endif //_MUD_UNIX_UTILL_H_

