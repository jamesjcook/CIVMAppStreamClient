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

#if __APPLE_CPP__ || __APPLE_CC__
#include <mach/clock.h>
#include <mach/mach.h>
#import "TargetConditionals.h"
#endif

namespace mud
{

/** Dummy method; only needed in Windows platform */
uint64_t TimeVal::getStaticFrequency()
{
    return 0;
}

/**
 *  Unix time value class.
 */

/** Get the current system time. */
TimeVal TimeVal::now()
{
    struct timeval timeVal;

    ::gettimeofday( &timeVal, 0 );

    return TimeVal( (uint64_t) (timeVal.tv_sec * 1000000ULL + timeVal.tv_usec) );
}

//Constructor.
TimeVal::TimeVal( bool boolCurTimeFlag )
{
    if (boolCurTimeFlag)
    {
        struct timeval timeVal;

        ::gettimeofday( &timeVal, 0 );

        mValue = timeVal.tv_sec * 1000000ULL + timeVal.tv_usec;
    }
    else
    {
        mValue = 0;
    }
}

/** Convert the time into broken-time. */
void TimeVal::toLocal( 
                uint32_t& year, 
                uint32_t& month, 
                uint32_t& day,
                uint32_t& hour,
                uint32_t& minute,
                uint32_t& second,
                uint32_t& milli ) const
{
    struct tm localTime;

    time_t tempTime = (time_t) (mValue / 1000000);

    localtime_r( &tempTime, &localTime );

    year = localTime.tm_year + 1900;
    month = localTime.tm_mon + 1;
    day = localTime.tm_mday;
    hour = localTime.tm_hour;
    minute = localTime.tm_min;
    second = localTime.tm_sec;
    milli = (uint32_t) ((mValue % 1000000) / 1000); 

}

} //namespace mud
