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

#include <windows.h>
#include <winbase.h>
#include <time.h>

#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000Ui64
#else
    #define DELTA_EPOCH_IN_MICROSECS  11644473600000000ULL
#endif
 
#include "..\TimeVal.h"

namespace mud
{

/**
 *  Windows time value class.
 */

/** For windows time frequency */
uint64_t TimeVal::getStaticFrequency()
{
    static long oneShot = -1;
    static uint64_t staticValue = 0;
               
    while (0 == staticValue)
    {
        if (InterlockedIncrement(&oneShot) == 0)
        {
            // Darek: QueryPerformanceFrequency() is constant for a given
            // Windows session (again, on EC2 it is always 62.5 MHz) so it is
            // not necessary to call it every single time you read the counter.
        
            LARGE_INTEGER largeFrequency;
            if ( ::QueryPerformanceFrequency( &largeFrequency ) )
            {
                // clock frequency
                staticValue = largeFrequency.QuadPart;
            }
            else
            {
                // highly unlikely event, but as fallback-measure
                // use hard-coded frequency
                staticValue = 62;
            }
            break;
        }
    }
               
    return staticValue;
}

/** Get the current system time. */
TimeVal TimeVal::now()
{
    FILETIME ft;
    unsigned __int64 tmpres = 0;

    GetSystemTimeAsFileTime( &ft );
 
    tmpres |= ft.dwHighDateTime;
    tmpres <<= 32;
    tmpres |= ft.dwLowDateTime;
 
    /*converting file time to unix epoch*/
    tmpres /= 10;  /*convert into microseconds*/
    tmpres -= DELTA_EPOCH_IN_MICROSECS; 
 
    return TimeVal( (uint64_t) (tmpres) );
}

/** Get the current mono time. */
TimeVal TimeVal::mono()
{
    LARGE_INTEGER largeCounter;

    QueryPerformanceCounter( &largeCounter );
    uint64_t const frequency = TimeVal::getStaticFrequency();

    return TimeVal( (uint64_t) (( largeCounter.QuadPart * 1000000 ) / frequency ) );
}

//Constructor.
TimeVal::TimeVal( bool boolCurTimeFlag )
{
    if (boolCurTimeFlag)
    {
        FILETIME ft;
        unsigned __int64 tmpres = 0;

        GetSystemTimeAsFileTime(&ft);
     
        tmpres |= ft.dwHighDateTime;
        tmpres <<= 32;
        tmpres |= ft.dwLowDateTime;
     
        /*converting file time to unix epoch*/
        tmpres /= 10;  /*convert into microseconds*/
        tmpres -= DELTA_EPOCH_IN_MICROSECS; 
     
        mValue = tmpres;
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

    localtime_s( &localTime, &tempTime );

    year = localTime.tm_year + 1900;
    month = localTime.tm_mon + 1;
    day = localTime.tm_mday;
    hour = localTime.tm_hour;
    minute = localTime.tm_min;
    second = localTime.tm_sec;
    milli = (uint32_t) ((mValue % 1000000) / 1000); 
}

} //namespace
