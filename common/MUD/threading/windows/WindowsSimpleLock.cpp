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

#include <assert.h>

#include "..\SimpleLock.h"

using namespace mud;

SimpleLock::SimpleLock() 
{
    CRITICAL_SECTION* pCriticalSection = new CRITICAL_SECTION;

    assert( pCriticalSection != NULL );

    InitializeCriticalSection( pCriticalSection );

    writeLock = pCriticalSection;
}

SimpleLock::~SimpleLock() 
{
    DeleteCriticalSection( (CRITICAL_SECTION*) writeLock );

    delete (CRITICAL_SECTION*) writeLock;
}

void SimpleLock::lock() const 
{
    EnterCriticalSection( (CRITICAL_SECTION*) writeLock );
}

void SimpleLock::unlock() const 
{
    LeaveCriticalSection( (CRITICAL_SECTION*) writeLock );
}
