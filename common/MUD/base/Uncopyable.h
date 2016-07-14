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

#ifndef _included_mud_Uncopyable_h_
#define _included_mud_Uncopyable_h_

namespace mud
{

/**
 * Simple base class for rendering a derived class uncopyable.
 */
class Uncopyable
{

protected:

    Uncopyable() {};

private:

    Uncopyable(const Uncopyable& rhs);
    Uncopyable& operator=(const Uncopyable& rhs);
};

} // namespace mud

#endif // _included_mud_Uncopyable_h_

