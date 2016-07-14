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

#include "XStx/client/XStxClientAPI.h"
#include <vector>
#include <windows.h>

class DumpFrameTest {

public:
    DumpFrameTest();
    ~DumpFrameTest();

    /**
     Parse the comma delimited environment variable to get times (in seconds) 
     to dump a frame.
    */
    void parseEnvironmentVariable(char* times);

    /**
     Update timestamps from frame that is to be drawn.
    */
    void setTimeStamps(const XStxRawVideoFrame* frame);

    /**
     Dump the frame into a text file at times specified in the environment var
    */
    void dumpFrame(uint32_t height, uint32_t width, HWND windowHandle);

private:
    bool mEnableAutomatedFrameDumping;
    bool mFirstFrame;
    uint64_t mBaseTimeStamp;
    uint64_t mLastTimeStamp;
    std::vector <int> mFrameDumpTimes;
};
