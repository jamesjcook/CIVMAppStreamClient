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

#ifndef _MUD_SIMPLE_LOCK_H_
#define _MUD_SIMPLE_LOCK_H_

#include "../base/Uncopyable.h"

namespace mud 
{

/**
    Provides access to a Mutex, in the current OS.
*/
class SimpleLock 
    :
    private Uncopyable
{
public:

    SimpleLock();
    virtual ~SimpleLock();

    /**
        Requests a lock
    */
    void lock() const;

    /**
        Unlocks the mutex.
    */
    void unlock() const;

protected:

    void * writeLock;
};

} //namespace mud

#endif //_MUD_SIMPLE_LOCK_H_
