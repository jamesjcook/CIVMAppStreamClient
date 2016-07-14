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

#ifndef _MUD_WAITABLE_LOCK_H_
#define _MUD_WAITABLE_LOCK_H_

#include <stdint.h> // for uint64_t
#include "../base/Uncopyable.h"

namespace mud 
{

/**
 * Object to allow locking a resource and waiting for changes to
 * that resource to be signalled.
 */
class WaitableLock
    :
    private Uncopyable
{
public:

    WaitableLock();

    virtual ~WaitableLock();

    /** Acquire the lock */
    void lock();

    /** Release the lock */
    void unlock();

    /**
     * Wait for the signal and acquire the lock.  The calling thread should NOT
     * already have the lock.
     */
    void waitForSignalAndLock();

    /**
     * Wait for the signal and acquire the lock. The calling thread should NOT
     * already have the lock. This call only waits until waitTimeInMs has
     * elasped. Inwhich case the function may return without being signaled.
     * @param waitTimeInMs the max relative time in milliseconds to wait
     * @return true if this lock has been signaled.
     */
    bool waitForSignalAndLock(uint64_t waitTimeInMs);

    /**
     * Set the signal.  The calling thread must be holding the lock.
     */
    void signal();

    /**
     * Set the signal.  The calling thread must be holding the lock.
     */
    void clearSignal();

private:

    void* data;
};

}; // namespace AmazonNetworkLib

#endif // _MUD_WAITABLE_LOCK_H_
