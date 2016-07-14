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

#ifndef _MUD_SCOPE_LOCK_H_
#define _MUD_SCOPE_LOCK_H_

#include "SimpleLock.h"
#include "../base/Uncopyable.h"

namespace mud 
{

/**
    A wrapper around SimpleLock, that uses the objects lifecycle
    to acquire and release the provided lock.
*/
class ScopeLock 
    :
    private Uncopyable
{
public:

    ScopeLock( const SimpleLock & sl ) : sl(sl) 
    {
        sl.lock();
    }

    ~ScopeLock() 
    {
        sl.unlock();
    }

private:

    const SimpleLock & sl;
};

}; // namespace mud

#endif //_MUD_SCOPE_LOCK_H_
