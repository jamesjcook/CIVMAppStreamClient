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

#ifndef _included_FixedSizePool_h_
#define _included_FixedSizePool_h_

#include "../threading/SimpleLock.h"
#include "../threading/ScopeLock.h"
#include "../base/SmartPointers.h"
#include "AmazonCompositeResult/SimpleResultCodes.h"
#include <set>
#include <list>
#include <iostream>
#include <assert.h>


namespace mud {

    /**
     * FixedSizePool is a preallocated pool of a fixed number of
     * elements that themselves are of fixed size. The user defines an
     * allocater/deallocator for the the element type and initialize
     * the pool with the desired number of allocated objects. The user
     * can then retrieve and reserve elements from the pool for use
     * and later return them to the pool without any restrictions on
     * order of return relative to retrieval order.
     */

    template<class T>
        class FixedSizePool
    {
    public:

        /** 
         * The user of the pool must provide an object implementing
         * FixedSizePool::Allocator so that the pool know how to
         * create the user's frame type.
         */
        class Allocator {
        public:
            virtual ~Allocator() {}

            /**
             * Provides an allocated element.  
             * 
             * @return a pointer to an allocated element on success,
             * NULL o.w.
             *
             */
            virtual T* allocate() = 0;    
        
            /**
             * Deallocate an element.
             *
             * @param[in] element the element to be deallocated.
             */
            virtual void deallocate(T* element) = 0;
        };

        FixedSizePool() {}
    
        ~FixedSizePool() {
            deallocate();
        }

        /**
         * Allocates a pool with num elements of type T.
         *   
         * @param[in] allocator a shared pointer to an allocator.  The FixedSizePool
         *            will copy the shared pointer.
         * @param[in] numElements the number of elements to allocate in the pool.
         * @return SIMPLE_RESULT_OK on success,
         *         SIMPLE_RESULT_INVALID if the allocator is NULL.
         *         SIMPLE_RESULT_NO_MEMORY otherwise.
         */
        int allocate(shared_ptr<Allocator> allocator, int numElements) {
        
            int result = SIMPLE_RESULT_OK;
        
            deallocate();
        
            T* element;
            mElementPoolMutex.lock();
            mAllocator = allocator;
            if(NULL == mAllocator.get()) {
                result = SIMPLE_RESULT_INVALID;
            } else {
                for(int i = 0; i < numElements; i++) {
                    if(NULL == (element = mAllocator->allocate())) {
                        result = SIMPLE_RESULT_NO_MEMORY;
                        break;
                    } else {
                        mAvailableElements.push_back(element);
                    }
                }
            }
            mElementPoolMutex.unlock();
            return result;
        }
  
        /**
         * Removes all elements from the pool.  Calls supplied
         * deallocate method on the elements as they are removed.
         * 
         * @return SIMPLE_RESULT_OK on success or 
         *         SIMPLE_RESULT_NOT_INITIALIZED if the pool has
         *         not been initialized.
         */
        int deallocate() {
            int result =  SIMPLE_RESULT_OK;
            typename std::set<T*>::iterator inUseIt;
            mElementPoolMutex.lock();
            if(NULL != mAllocator) {
                if(!mElementsInUse.empty()) {
                    for(inUseIt = mElementsInUse.begin(); inUseIt != mElementsInUse.end(); 
                        inUseIt++) {
                        mAllocator->deallocate(*inUseIt);
                    }
                    mElementsInUse.clear();
                }
                while(mAvailableElements.empty() == false) { 
                    mAllocator->deallocate(mAvailableElements.front());
                    mAvailableElements.pop_front();
                }
                mAllocator.reset();
            } else {
                result =  SIMPLE_RESULT_NOT_INITIALIZED;
            }
            mElementPoolMutex.unlock();
            return result;
        }

        /**
         * Reserves an element from the buffer pool for use.  The element must be
         * returned to the pool via a call to recycleElement when the caller
         * no longer needs the element.
         * @param[out] element contains the element upon successful return or NULL
         *             if failed to allocate.
         * @return SIMPLE_RESULT_OK on success or
         *         SIMPLE_RESULT_PENDING if no buffers available.
         *         SIMPLE_RESULT_NOT_INITIALIZED if the pool has
         *         not been initialized.
         */
        int getElement(T*& element)  {
            int result = SIMPLE_RESULT_OK;
            element = NULL;
            mElementPoolMutex.lock();
            if(NULL == mAllocator) {
                result = SIMPLE_RESULT_NOT_INITIALIZED;
            } else if(mAvailableElements.empty() == false) {
                element = mAvailableElements.front();
                mAvailableElements.pop_front();
                mElementsInUse.insert(element);
            } else {
                result = SIMPLE_RESULT_PENDING;
            }
            mElementPoolMutex.unlock();
            return result;
        }

        /**
         * Returns a element to the buffer pool.
         *  
         * @param[in] element to be returned.  
         * @return SIMPLE_RESULT_OK if the element could be returned to the pool 
         *         SIMPLE_RESULT_NOT_FOUND if the element isn't recognized as
         *         coming from the pool.
         *         SIMPLE_RESULT_NOT_INITIALIZED if the pool has
         *         not been initialized.
         */
        int recycleElement(T* element) {
            int result = SIMPLE_RESULT_OK;
            mElementPoolMutex.lock();
            if (NULL == mAllocator) {
                result = SIMPLE_RESULT_NOT_INITIALIZED;
            } else if (mElementsInUse.find(element) == mElementsInUse.end()) {
                result = SIMPLE_RESULT_NOT_FOUND;
            } else {
                mElementsInUse.erase(element);
                mAvailableElements.push_front(element);
            }
            mElementPoolMutex.unlock();
            return result;
        }
  
        /**
         * Check if given element belongs to this pool
         * @param[in] element to be checked
         * @return TRUE if it belongs to the pool, FALSE if not
         */
        bool isInUse(T* element) const {
            bool result = true;
            ScopeLock scope(mElementPoolMutex);
            if ( NULL == mAllocator || 
                 mElementsInUse.find(element) == mElementsInUse.end() )
            {
                result = false;
            }
            return result;
        }

    private:
  
        // The mutex allows protects getElement and recycleElement so that they
        // are thread safe.
        SimpleLock    mElementPoolMutex;
        std::list<T*> mAvailableElements;
        std::set<T*>  mElementsInUse;
        shared_ptr<Allocator>  mAllocator;
    };

} // end namespace
#endif
