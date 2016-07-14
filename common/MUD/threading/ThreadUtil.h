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

#ifndef _MUD_THREAD_UTIL_H_
#define _MUD_THREAD_UTIL_H_

#include "../base/TimeVal.h"

namespace mud 
{ 

class ThreadUtil 
{
public:
    /**
     * Asks the OS to increase the timer accuracy. On Windows, this
     * function will set the timer accuracy to 1 ms for all processes
     * on the machine.  A call to setAccurateTiming should be paired
     * with a call to unsetAccurateTiming so the scheduler can return
     * to its default behavior.
     *
     * NOTE: So far only implemented for Windows
     */
    static void setAccurateTiming();

    /**
     * Asks the OS to end the high timer accuracy setting that was
     * previously set in a setAccurateTiming call.
     *
     * NOTE: So far only implemented for Windows
     */
    static void unsetAccurateTiming();

    /**
     * Asks the OS to sleep for the approximate time given.
     * @param milliseconds the time, in milliseconds to sleep for. 
     */
    static void sleep( unsigned long milliseconds );

    /**
     * Yields the CPU to other threads.
     */
    static void yield( );

    /**
     * Attempts to sleep with better accuracy then 'sleep'.
     * This is done by only yielding the CPU for sleep amounts 
     * lower than a specific interval.
     * @param milliseconds the time, in milliseconds to sleep/wait for.
     */ 
    static void accurateSleep( unsigned long milliseconds )
    {
        // the minimum time we are allowed to try to sleep()
        static const uint64_t MIN_SLEEP = 1 * 1000;
        TimeVal now = TimeVal::mono();
        TimeVal stopAt = now + TimeVal::fromMilliSeconds(milliseconds);
        for (;;) {
            now = TimeVal::mono();
            if (now >= stopAt) {
                return;
            }
            uint64_t delta = stopAt.toMicroSeconds() - now.toMicroSeconds();
            if (delta > MIN_SLEEP) {
                sleep(1);
            }
        }
    }
};

}; // namespace mud

#endif // _MUD_THREAD_UTIL_H_
