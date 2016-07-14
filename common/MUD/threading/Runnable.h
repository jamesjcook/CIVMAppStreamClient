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

#ifndef _MUD_RUNNABLE_H_
#define _MUD_RUNNABLE_H_

namespace mud
{ 

/**
 * Interface to an object that can be "run", typically by a Thread object.
 */
class Runnable 
{
public:
    virtual ~Runnable()
    { }

    virtual void run() = 0;
};

}; // namespace mud

#endif //_MUD_RUNNABLE_H_
