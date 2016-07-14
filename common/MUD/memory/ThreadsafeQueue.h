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

#ifndef _MUD_THREADSAFE_QUEUE_H_
#define _MUD_THREADSAFE_QUEUE_H_

/**
 * Queue of objects that allows a producer thread to push items on the queue and
 * a consumer thread to (optionally) block until there is something to pop from
 * the queue.
 */

#include <assert.h>
#include <deque>
#include "../threading/WaitableLock.h"
#include "../base/TimeVal.h"

namespace mud 
{

template< class ElementType >
class ThreadsafeQueue
{
public:

    /**
     * Create a new ThreadsafeQueue instance.  maxCapacity is used to set a firm
     * limit to the number of items that the queue will contain.
     */
    ThreadsafeQueue(size_t maxCapacity)
        :
        m_maxCapacity(maxCapacity)
    {
    }

    /**
     * Destroy a instance of ThreadsafeQueue.  Derivations of this template are
     * responsible for emptying the queue before we get here.
     */
    virtual ~ThreadsafeQueue() 
    {
        assert(m_deque.empty());
    }

    /**
     * Push a new item on to the queue.  If the queue is already at maxCapacity
     * the push operation is aborted and this method returns false.  Otherwise
     * this method returns true.
     */
    bool push(ElementType element)
    {
        bool pushed = false;

        m_lock.lock();
        if (m_deque.size() < m_maxCapacity)
        {
            m_deque.push_back(element);
            pushed = true;
        }
        m_lock.signal();
        m_lock.unlock();

        return pushed;
    }

    /**
     * Pop an item from the queue, returning it in "element".  If the queue is
     * empty, "element" is unchanged and this method returns false.  Otherwise
     * this method returns true.
     */
    bool pop(ElementType& element)
    {
        bool popped = false;

        m_lock.lock();
        if(!m_deque.empty())
        {
            element = m_deque.front();
            m_deque.pop_front();
            popped = true;
            if(m_deque.empty()) {
                m_lock.clearSignal();
            }
        }
        m_lock.unlock();

        return popped;
    }

    /**
     * Interrupt a waiting consumer thread (for things like allowing it to exit
     * so that teardown can complete).
     */
    void interrupt()
    {
        m_lock.lock();
        m_lock.signal();
        m_lock.unlock();
    }

    /**
     * Pop the next item in the queue, waiting of necessary.  If the queue is
     * signalled but empty this method returns false and "element" is unchanged.
     * Otherwise "element" contains the popped item and this method returns
     * true.
     */
    bool waitAndPop(ElementType& element)
    {
        if (pop(element))
        {
            return true;
        }

        m_lock.waitForSignalAndLock();
        bool empty = m_deque.empty();
        if (!empty)
        {
            element = m_deque.front();
            m_deque.pop_front();
        } 
        else 
        {
            m_lock.clearSignal();
        }
        m_lock.unlock();

        return !empty;
    }

    bool front(ElementType & element) {
        bool hasFirst = false;

        m_lock.lock();
        if(!m_deque.empty())
        {
            element = m_deque.front();
            hasFirst = true;
        }
        m_lock.unlock();
        return hasFirst;
    }

    bool waitForFront(ElementType & element) {
        if (front(element))
        {
            return true;
        }
        m_lock.waitForSignalAndLock();
        bool empty = m_deque.empty();
        if (!empty)
        {
            element = m_deque.front();
        }
        else 
        {
            m_lock.clearSignal();
        }
        m_lock.unlock();
        return !empty;
    }


    bool waitForFront(ElementType & element, TimeVal maxTime) {
        if (front(element))
        {
            return true;
        }
        bool empty = true;
        if (m_lock.waitForSignalAndLock(maxTime.toMilliSeconds()))
        {
            empty = m_deque.empty();
            if (!empty)
            {
                element = m_deque.front();
            } 
            else 
            {
                m_lock.clearSignal();
            }
        }
        m_lock.unlock();
        return !empty;
    }

    /**
     * @return true iff the queue is empty.
     */
    bool empty() 
    {
        m_lock.lock();
        bool result = m_deque.empty();
        m_lock.unlock();
        return result;
    }

    /**
     * @return the size of the queue.
     */
    size_t size()
    {
        m_lock.lock();
        size_t result = m_deque.size();
        m_lock.unlock();
        return result;
    }

private:

    WaitableLock m_lock;

    size_t m_maxCapacity;

    std::deque< ElementType > m_deque;
};

} // namespace mud

#endif //_MUD_THREDSAFE_QUEUE_H_
