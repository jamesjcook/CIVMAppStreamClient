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

#include "..\ThreadUtil.h"

using namespace mud;

void ThreadUtil::setAccurateTiming()
{
  timeBeginPeriod(1);
}

void ThreadUtil::unsetAccurateTiming()
{
  timeEndPeriod(1);
}

void ThreadUtil::sleep( unsigned long milliseconds ) 
{
    ::Sleep( milliseconds );
}

void ThreadUtil::yield( )
{

    /* According to Darek M.: The official Microsoft documented way to
       fairly and efficiently yield a thread in Win32 is to call
       SwitchToThread() which despite the name takes no parameters, it
       is similar to the old Windows 3.x Yield() call. */

    SwitchToThread();  

}
