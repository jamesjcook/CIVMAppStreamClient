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

/**
  @class  DumpFrameTest

  @brief  Used for automated testing, dumps frames at times specified
          in an environment variable.
*/

#include "DumpFrameTest.h"
#include "GDICapture.h"
#include "Util.h"
#include <algorithm>
#include <fstream>

DumpFrameTest::DumpFrameTest() :
    mEnableAutomatedFrameDumping(false),
    mFirstFrame(true),
    mBaseTimeStamp(0),
    mLastTimeStamp(0)
{
}

DumpFrameTest::~DumpFrameTest()
{
}


void DumpFrameTest::parseEnvironmentVariable(char* times)
{
    mEnableAutomatedFrameDumping = true;  // used during timestamp update and frame dump
    char* token = strtok(times, " ,");
    while (token != NULL)
    {
        mFrameDumpTimes.push_back(atoi(token));
        token = strtok(NULL, " ,");
    }

    std::sort(mFrameDumpTimes.begin(), mFrameDumpTimes.end());  // sort in ascending order
}

void DumpFrameTest::setTimeStamps(const XStxRawVideoFrame* frame)
{
    if (mEnableAutomatedFrameDumping == true)
    {
        if (mFirstFrame == true && frame->mTimestampUs > 0)
        {
            mBaseTimeStamp = frame->mTimestampUs;
            mLastTimeStamp = frame->mTimestampUs;
            mFirstFrame = false;
        }
        else if (mFirstFrame == false)
        {
            mLastTimeStamp = frame->mTimestampUs;
        }
    }
}

void DumpFrameTest::dumpFrame(uint32_t height, uint32_t width, HWND windowHandle)
{
    if (mEnableAutomatedFrameDumping == true)
    {
        uint64_t totalTimePassedSeconds = (mLastTimeStamp - mBaseTimeStamp) / 1000000;
        static unsigned int nextExpectedIndex = 0;

        if (totalTimePassedSeconds == mFrameDumpTimes[nextExpectedIndex])
        {
            nextExpectedIndex++;
            if (nextExpectedIndex >= mFrameDumpTimes.size())
            {
                mEnableAutomatedFrameDumping = false;  // checked all the times, stop frame dumps
            }
            GDICapture GDI_instance(windowHandle);  // create GDICapture instance, pass window handle
            const unsigned char* data = GDI_instance.capture();  // get pixel data from window

            char file_name[25];
            snprintf(file_name, 25, "frame_data_%d.txt", totalTimePassedSeconds);
            std::ofstream pixel_file(file_name, std::ios::binary);  // create file stream and open as binary
            printf("Dumping pixel data (%d seconds elapsed)\n", totalTimePassedSeconds);
            pixel_file.write((const char*)data, height * width * 4);  // write all bytes in window (4 bytes per pixel)
            pixel_file.close();
        }
    }
}