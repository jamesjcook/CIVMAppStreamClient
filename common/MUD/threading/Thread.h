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

#ifndef _MUD_THREAD_H_
#define _MUD_THREAD_H_

#include <string>
#include <stdint.h>
#include "Runnable.h"
#include "../base/Uncopyable.h"

namespace mud 
{ 

/**
 * A simple thread class.
 */
class Thread
    :
    private Runnable,
    private Uncopyable
{
public:
    /** 
     *  Get the ID of the calling thread.
     *  @return the ID of the calling thread as an opaque pointer.
     */
    static void* getId();

public:
    
    /**
     * Thread object ctor
     * @param name A name for this Thread
     *
     * The new thread executes this instances run() method
     */
    Thread( const char* aName );

    /**
     * Thread object ctor
     * @param name A name for this Thread
     * @param runnable An object whose run() method will be invoked in the
     * new thread.  The caller retains ownership of
     * the Runnable instance.
     */
    Thread( const char* name, Runnable& runnable );

    /**
     * Thread object dtor.  Blocks until this Thread stops (if it started).
     */
    virtual ~Thread();

    /**
     * Kick off the thread.
     * @return An Amazon Simple Result Code.
     */
    uint32_t start();

    /**
     * Wait for this Thread to stop (if it started).
     * @return An Amazon Simple Result Code.
     */
    uint32_t join();

    /**
     * Get the name of the thread.
     * @return The name of the thread.
     */
    const char* getName();

protected:
    /** The thread procedure. */
    void run();

private:                 
    // Somewhere to hang implementation specific data.
    void* data;

    /**
     * Disable copy-constructor and = operator
     * by declaring them private and not providing implementation.
     */
    Thread( const Thread& other );
    Thread& operator=( const Thread& other );
};

} // namespace mud

/**
 * Define a derivative of Thread that will invoke
 * a specific method of a specific instance in
 * the new thread.
 *
 * @param[in] name Name of the resulting derived class
 * @param[in] type The type of object whose methos is to be invoked
 * @param[in] method The name of the method to invoke
 */
#define DEFINE_METHOD_THREAD( name, type, method )      \
                                                        \
    class name : public mud::Thread                     \
    {                                                   \
    public:                                             \
        name(                                           \
            const char* aName,                          \
            type& aInstance)                            \
            :                                           \
            mud::Thread( aName ),                       \
            instance( aInstance )                       \
        {                                               \
        }                                               \
                                                        \
        void run()                                      \
        {                                               \
            instance.method();                          \
        }                                               \
                                                        \
    private:                                            \
        type& instance;                                 \
    };

#endif // _MUD_THREAD_H_
