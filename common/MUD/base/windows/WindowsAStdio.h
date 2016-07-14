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

#ifndef _MUD_WINDOWSASTDIO_H
#define _MUD_WINDOWSASTDIO_H


#ifndef va_copy
    #define va_copy(d,s) ((d) = (s))
#endif

int mud::AStdio::vsnprintf(char * s, size_t n, const char * format, va_list arg) {
    va_list arg_copy;
    va_copy(arg_copy, arg);
    int amountNeeded = _vscprintf(format, arg_copy);
    va_end(arg_copy);
    if (amountNeeded < 0) {
        return amountNeeded;
    }
    // unfortunatly the return codes from this function are not great.
    // if we set _TRUNCATE then -1 will be returned. However, that is the same
    // error code for buffer being null, format being null, or count be < 0
    // Furthmore, it doesn't appear that errno gets set to ERANGE, unless we remove
    // TRUNCATE option.
    // http://msdn.microsoft.com/en-us/library/d3xd30zz%28v=vs.80%29.aspx
    if (s == NULL) {
        return - 1;
    }
    if (n <= 0) {
        return -1;
    }
    if (format == NULL) {
        return -1;
    }
    vsnprintf_s(s, n, _TRUNCATE, format, arg);
    return amountNeeded;
}


#endif
