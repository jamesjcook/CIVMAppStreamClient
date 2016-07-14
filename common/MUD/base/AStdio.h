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

#ifndef _MUD_ASTDIO_H
#define _MUD_ASTDIO_H

#include <stdio.h>
#include <stdarg.h>

namespace mud
{

/** Providing more crossplatform constistency for stdio operations.
*/
class AStdio {
public:

    /** @see http://www.cplusplus.com/reference/cstdio/vsnprintf/
    * @param s Pointer to the buffer to store the printed string into.
    * @param n the size of the buffer 's'.
    * @param format the c string format string
    * @param arg the variable argument list to print with the format.
    * @return -1 on error. The amount of characters printed (not including the null
    *    character) into s. If this is bigger then 'n' then the message was truncated.
    */
    static inline int vsnprintf(char * s, size_t n, const char * format, va_list arg);

    /** @see vsnprintf */
    static inline int snprintf(char * s, size_t n, const char * format, ...)
    {
        va_list argList;
        va_start(argList, format);
        int ret = vsnprintf(s, n, format, argList);
        va_end(argList);
        return ret;
    }

private:
    AStdio();
};

} // namespace



#if defined(__linux__) || (__APPLE_CPP__ || __APPLE_CC__)
    #include "unix/UnixAStdio.h"
#elif defined(_WIN32)
    #include "windows/WindowsAStdio.h"
#endif

#endif
