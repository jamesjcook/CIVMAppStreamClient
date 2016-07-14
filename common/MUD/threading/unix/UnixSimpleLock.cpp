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

#include "../SimpleLock.h"

using namespace mud;

//The helper structure.
struct PThreadRecursiveLock 
{
    pthread_mutex_t writeLock;
    pthread_mutexattr_t writeLockAttribute;
};

SimpleLock::SimpleLock() 
{
    PThreadRecursiveLock * recurLock = new PThreadRecursiveLock;

    pthread_mutexattr_init( &(recurLock->writeLockAttribute) );

    pthread_mutexattr_settype( &(recurLock->writeLockAttribute), PTHREAD_MUTEX_RECURSIVE );

    pthread_mutex_init( &(recurLock->writeLock), &(recurLock->writeLockAttribute) );

    writeLock = recurLock;
}

SimpleLock::~SimpleLock() 
{
    PThreadRecursiveLock * recurLock = (PThreadRecursiveLock *) writeLock;

    pthread_mutex_destroy( &(recurLock->writeLock) );

    pthread_mutexattr_destroy( &(recurLock->writeLockAttribute) );

    delete recurLock;
}

void SimpleLock::lock() const 
{
    pthread_mutex_lock( &(((PThreadRecursiveLock *) writeLock)->writeLock) ) ;
}

void SimpleLock::unlock() const 
{
    pthread_mutex_unlock( &(((PThreadRecursiveLock *) writeLock)->writeLock) );
}
