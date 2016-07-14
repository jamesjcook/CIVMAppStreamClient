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

/*
 * ISynchSource.h
 *
 *  Created on: Aug 14, 2013
 *      Author: ykim
 */
#ifndef ISYNCHSOURCE_XSTX_H_
#define ISYNCHSOURCE_XSTX_H_

#include <stdint.h>

class ISynchSourceXStx {
public:

  static const uint64_t INVALID_TIMESTAMP = -1;

  virtual ~ISynchSourceXStx() {}

  /**
   * Returns the current program time. Must be thread safe.
   * @return current program time in Us.
   **/
  virtual uint64_t getCurrentTimeUs() = 0;


};
#endif /* ISYNCHSOURCE_XSTX_H_ */
