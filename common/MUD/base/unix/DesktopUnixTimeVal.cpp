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

#include <time.h>
#include <sys/time.h>

#include "../TimeVal.h"

namespace mud
{

/**
 *  Unix time value class.
 */

/** Get the current mono time. */
TimeVal TimeVal::mono()
{
    struct timespec timeSpec;

    ::clock_gettime(CLOCK_MONOTONIC, &timeSpec);
 
    return TimeVal( (uint64_t) (timeSpec.tv_sec * 1000000ULL + timeSpec.tv_nsec / 1000ULL) );
}

} //namespace mud
