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

#include <errno.h>
#include <pthread.h>

#include "AmazonCompositeResult/SimpleResultCodes.h"

#include "../Thread.h"
#include "../ScopeLock.h"

#include "../../base/unix/UnixUtil.h"

using namespace acr;
using namespace mud;

/**
 * Unix implementation of Thread.
 */
class UnixThread 
{

public:
    UnixThread( const char* aName, Runnable& aRunnable )
        :
        name( aName ),
        runnable( aRunnable ),
        isStarted( false ),
        isRunning( false ) 
    {
    }

    ~UnixThread() 
    {
        join();
    }

    uint32_t start() 
    {
        ScopeLock scope( lock );

        if (isStarted) 
        {
            return SIMPLE_RESULT_ALREADY_STARTED;
        }

        int result = pthread_create( &id, NULL, staticThreadProc, this );

        if (result != 0) 
        {
            return UnixUtil::errnoToSimpleResultCode( result );
        }

        isStarted = true;
        isRunning = true;

        return SIMPLE_RESULT_OK;
    }

    uint32_t join() 
    {
        {
            ScopeLock scope( lock );

            if (!isStarted) 
            {
                return SIMPLE_RESULT_NOT_STARTED;
            }

            if (!isRunning) 
            {
                return SIMPLE_RESULT_OK;
            }
        }

        int result = pthread_join( id, NULL );
    
        if (result != 0 && result != ESRCH) 
        {
            return UnixUtil::errnoToSimpleResultCode( result );
        }
    
        return SIMPLE_RESULT_OK;
    }    

    const char* getName()
    {
        return name.c_str();
    }

private:

    static void* staticThreadProc( void* param ) 
    {
        ((UnixThread*)param)->threadProc();

        return NULL;
    }

    void threadProc() 
    {
        runnable.run();

        {
            ScopeLock scope( lock );
            isRunning = false;
        }
    }

    std::string name;
    Runnable& runnable;
    SimpleLock lock;
    bool isStarted;
    bool isRunning;
    pthread_t id;
};

void* Thread::getId()
{
    return (void*) pthread_self();
}

Thread::Thread( const char* aName )
    :
    data(new UnixThread( aName, *this )) 
{

}

Thread::Thread( const char* aName, Runnable& aRunnable )
    :
    data(new UnixThread( aName, aRunnable )) 
{
}

Thread::~Thread() 
{
    delete ((UnixThread*)data);
}

uint32_t Thread::start() 
{
    return ((UnixThread*)data)->start();
}

uint32_t Thread::join() 
{
    return ((UnixThread*)data)->join();
}

/** Get the name of the thread. */
const char* Thread::getName()
{
    return ((UnixThread*)data)->getName();
}

void Thread::run() 
{
}

