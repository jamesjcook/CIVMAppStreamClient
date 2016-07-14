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

#include <assert.h>
#include <windows.h>

#include "../WaitableLock.h"
#include "../ScopeLock.h"
#include "../../base/TimeVal.h"

using namespace mud;

/**
 * Windows implementation of WaitableLock.
 */
class WindowsWaitableLock 
{
public:
    WindowsWaitableLock() 
    {
        event = CreateEvent( NULL, TRUE, FALSE, NULL );
    }

    ~WindowsWaitableLock()
    {
        CloseHandle( event );
    }

    void lock() 
    {
        cs.lock();
    }

    void unlock() 
    {
        cs.unlock();
    }

    void waitForSignalAndLock()
    {
        /**
         * After getting the signal and acquiring the lock we check that
         * the signal is still set (because some other caller to
         * waitForSignalAndLock may have snuck in between the
         * wait and lock. Note: waitForSingleObject does not release
         * the lock held in the lock call).
         * If the signal is cleared we release the lock and go back to waiting.
         */
        for (;;) 
        {
            DWORD waitResult = WaitForSingleObject( event, INFINITE );

            assert( waitResult == WAIT_OBJECT_0 );
            
            lock();

            waitResult = WaitForSingleObject( event, 0 );
            
            if (WAIT_OBJECT_0 == waitResult) 
            {
                clearSignal();

                return;
            }

            assert( waitResult == WAIT_TIMEOUT );
            
            unlock();
        }
    }
    bool waitForSignalAndLock(uint64_t originalWaitTime) {
        TimeVal startedAt = TimeVal::mono();
        uint64_t waitTimeInMs = originalWaitTime;
        for (;;) {

            DWORD waitResult = WaitForSingleObject(event, (DWORD)(waitTimeInMs));
            lock();

            DWORD objectIsSet = WaitForSingleObject(event, 0);
            if (objectIsSet == WAIT_OBJECT_0) {
                clearSignal();
                return true;
            }
            if (waitResult == WAIT_TIMEOUT) {
                return false;
            }
            uint64_t elapsed = startedAt.elapsedMono().toMilliSeconds();
            if (elapsed >= waitTimeInMs) {
                return false; // huh.. 
            }
            waitTimeInMs = (uint64_t) (originalWaitTime - elapsed);
            unlock();
        }
    }

    void signal() 
    {
        SetEvent( event );
    }

    void clearSignal() 
    {
        ResetEvent( event );
    }

private:
    SimpleLock cs;
    HANDLE event;
};

WaitableLock::WaitableLock()
    :
    data(new WindowsWaitableLock()) 
{
}

WaitableLock::~WaitableLock() 
{
    delete ((WindowsWaitableLock*)data);
}

void WaitableLock::lock() 
{
    ((WindowsWaitableLock*)data)->lock();
}

void WaitableLock::unlock() 
{
    ((WindowsWaitableLock*)data)->unlock();
}

void WaitableLock::waitForSignalAndLock() 
{
    ((WindowsWaitableLock*)data)->waitForSignalAndLock();
}

bool WaitableLock::waitForSignalAndLock(uint64_t waitTimeInMs) 
{
   return ((WindowsWaitableLock*)data)->waitForSignalAndLock(waitTimeInMs); 
}


void WaitableLock::signal() 
{
    ((WindowsWaitableLock*)data)->signal();
}

void WaitableLock::clearSignal() 
{
    ((WindowsWaitableLock*)data)->clearSignal();
}

