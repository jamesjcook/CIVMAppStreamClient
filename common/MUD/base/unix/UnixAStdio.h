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

#ifndef _MUD_UNIXASTDIO_H
#define _MUD_UNIXASTDIO_H


int mud::AStdio::vsnprintf(char * s, size_t n, const char * format, va_list arg) {
    return ::vsnprintf(s, n, format, arg);
}


#endif
