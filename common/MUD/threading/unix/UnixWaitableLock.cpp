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

#include <pthread.h>

#include "../WaitableLock.h"
#include "../../base/TimeVal.h"
#include <cstdio>

using namespace mud;

/**
 * Unix implementation of WaitableLock.
 */
class UnixWaitableLock
{
public:

    UnixWaitableLock()
        :
        signalled( false )
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_condattr_init(&cond_attributes);
#ifdef CLOCK_MONOTONIC
        //CLOCK_MONOTONIC and the pthread_condattr_setclock don't exist in iOS so
        // skip this call
#ifndef ANDROID
        // pthread_condattr_setclock doesn't exist on Android, but CLOCK_MONOTONIC
        // does. Sigh.
        pthread_condattr_setclock(&cond_attributes, CLOCK_MONOTONIC);
#endif
#endif
        pthread_cond_init(&cond, &cond_attributes);
    }

    ~UnixWaitableLock()
    {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&cond);
    }

    void lock()
    {
        pthread_mutex_lock(&mutex);
    }

    void unlock()
    {
        pthread_mutex_unlock(&mutex);
    }

    void waitForSignalAndLock()
    {
        lock();

        while (!signalled)
        {
            pthread_cond_wait(&cond, &mutex);
        }

        signalled = false;
    }
    bool waitForSignalAndLock(uint64_t waitTimeInMs) {
        lock();
        if (signalled) {
            signalled = false;
            return true;
        }
#ifdef CLOCK_MONOTONIC
        //Regular Unix should use mono since we set the pthread to use that clock
        // get the total sleep time as one value
        mud::TimeVal sleepTill = mud::TimeVal::mono() +
            mud::TimeVal::fromMilliSeconds(waitTimeInMs);
#else
        //iOS will use now() since mono() isn't based on system clock
        // get the total sleep time as one value
        mud::TimeVal sleepTill = mud::TimeVal::now() +
            mud::TimeVal::fromMilliSeconds(waitTimeInMs);
#endif
        // construct the timespec
        struct timespec timeSpec;
        timeSpec.tv_sec = sleepTill.toSeconds();
        timeSpec.tv_nsec = (sleepTill.toMicroSeconds() % 1000000ULL) * 1000ULL;
#ifdef ANDROID
        pthread_cond_timedwait_monotonic_np(&cond,&mutex,&timeSpec);
#else
        pthread_cond_timedwait(&cond, &mutex, &timeSpec);
#endif
        bool wasSignaled = signalled;
        signalled = false;
        return wasSignaled;
    }

    void signal()
    {
        signalled = true;
        pthread_cond_signal(&cond);
    }

    void clearSignal()
    {
        signalled = false;
    }

private:
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    pthread_condattr_t cond_attributes;
    bool signalled;
};

WaitableLock::WaitableLock()
    :
    data( new UnixWaitableLock() )
{
}

WaitableLock::~WaitableLock()
{
    delete ((UnixWaitableLock*)data);
}

void WaitableLock::lock()
{
    ((UnixWaitableLock*)data)->lock();
}

void WaitableLock::unlock()
{
    ((UnixWaitableLock*)data)->unlock();
}

void WaitableLock::waitForSignalAndLock()
{
    ((UnixWaitableLock*)data)->waitForSignalAndLock();
}

bool WaitableLock::waitForSignalAndLock(uint64_t waitTimeInMs)
{
   return ((UnixWaitableLock*)data)->waitForSignalAndLock(waitTimeInMs);
}

void WaitableLock::signal()
{
    ((UnixWaitableLock*)data)->signal();
}

void WaitableLock::clearSignal()
{
    ((UnixWaitableLock*)data)->clearSignal();
}

