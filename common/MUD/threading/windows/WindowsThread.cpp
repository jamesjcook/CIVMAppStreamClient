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

#include "..\Thread.h"
#include "..\ScopeLock.h"

#include "..\..\base\windows\WindowsUtil.h"

#ifdef XSTX_MUD_LOGGING
#include "..\..\logging\log.h"
static const char* TAG = "WindowsThread";
#endif // XSTX_MUD_LOGGING

using namespace mud;


/**
 * Windows implementation of Thread.
 */
class WindowsThread 
{
public:
    WindowsThread( const char* aName, Runnable& aRunnable ) 
        :
        name( aName ),
        runnable( aRunnable ),
        thread( NULL ) 
    {
    }

    ~WindowsThread() 
    {
        join();

        if(thread != NULL) 
        {
            CloseHandle( thread );
        }
    }

    uint32_t start() 
    {
        ScopeLock scope( lock );

        if (thread != NULL) 
        {
            return SIMPLE_RESULT_ALREADY_STARTED;
        }

        thread = CreateThread( NULL, 0, staticThreadProc, this, 0, NULL );

        if (NULL == thread) 
        {
            return WindowsUtil::systemErrorCodeToSimpleResultCode( GetLastError() );
        }

        return SIMPLE_RESULT_OK;;
    }

    uint32_t join() 
    {
        {
            ScopeLock scope(lock);

            if (NULL == thread) 
            {
                return SIMPLE_RESULT_NOT_STARTED;
            }
        }

        DWORD waitResult = WaitForSingleObject( thread, INFINITE );
    
        if (waitResult != WAIT_OBJECT_0) 
        {
            return WindowsUtil::systemErrorCodeToSimpleResultCode( GetLastError() );
        }
    
        return SIMPLE_RESULT_OK;
    }

    const char* getName()
    {
        return name.c_str();
    }

    static DWORD WINAPI staticThreadProc( void* param ) 
    {
        ((WindowsThread*)param)->threadProc();
        return 0;
    }

    void threadProc() 
    {
#ifdef XSTX_MUD_LOGGING
        LoggerPtr log = DefaultLogger::getCurrent();
        if (log.get() != NULL)
        {
            log->info(TAG, "--> threadProc() \"%s\"", name.c_str());
        }
        log.reset();
#endif // XSTX_MUD_LOGGING

        runnable.run();

#ifdef XSTX_MUD_LOGGING 
        log = DefaultLogger::getCurrent();
        if (log.get() != NULL)
        {
            log->info(TAG, "<-- threadProc() \"%s\"", name.c_str());
        }
#endif // XSTX_MUD_LOGGING
    }

private:
    std::string name;
    Runnable& runnable;
    SimpleLock lock;
    HANDLE thread;
};

void* Thread::getId()
{
    return (void*) GetCurrentThreadId();
}

Thread::Thread( const char* aName )
    :
    data(new WindowsThread( aName, *this )) 
{
}

Thread::Thread( const char* aName, Runnable& aRunnable )
    :
    data(new WindowsThread( aName, aRunnable )) 
{
}

Thread::~Thread() 
{
    delete ((WindowsThread*)data);
}

uint32_t Thread::start() 
{
    return ((WindowsThread*)data)->start();
}

uint32_t Thread::join() 
{
    return ((WindowsThread*)data)->join();
}

/** Get the name of the thread. */
const char* Thread::getName()
{
    return ((WindowsThread*)data)->getName();
}

void Thread::run() 
{
}

