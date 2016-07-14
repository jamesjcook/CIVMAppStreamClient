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

#include "../TimeVal.h"

#include <QuartzCore/QuartzCore.h>

#import "TargetConditionals.h"

namespace mud
{

/**
 *  Unix time value class.
 */

/** Get the current mono time. */
TimeVal TimeVal::mono()
{
    //Use CACurrentMediaTime since that is an extremely fast way to get the clock
    CFTimeInterval currTime = CACurrentMediaTime();
    
    return TimeVal::fromMicroSeconds(currTime * 1000000);
}


} //namespace mud
