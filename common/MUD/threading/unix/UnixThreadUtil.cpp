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

#include <unistd.h>
#include <sched.h>

#include "../ThreadUtil.h"

using namespace mud;

void ThreadUtil::setAccurateTiming()
{
}

void ThreadUtil::unsetAccurateTiming()
{
}

void ThreadUtil::sleep( unsigned long milliseconds ) 
{
    usleep( milliseconds * 1000 );
}

void ThreadUtil::yield( )
{
    sched_yield( );
}
