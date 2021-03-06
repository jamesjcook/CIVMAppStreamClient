/*
 * Copyright 2013-2014 Amazon.com, Inc. or its affiliates. All Rights
 * Reserved.
 *
 * Licensed under the Amazon Software License (the "License"). You may
 * not use this file except in compliance with the License. A copy of
 * the License is located at
 *
 * http://aws.amazon.com/asl/
 *
 * This Software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
 * OR CONDITIONS OF ANY KIND, express or implied. See the License for
 * the specific language governing permissions and limitations under
 * the License.
 *
 */


#include "AppStreamClientFileInput.h"
#include "AppStreamConnectionDefaults.h"
#include "XStx/common/XStxUtil.h"
#include <iostream>
#include <fstream>
#include <algorithm>

#define FILE_INPUT_SUCCESS 0
#define FILE_INPUT_ERROR -1

XStxExampleClientFileInput::XStxExampleClientFileInput(const char* fileName)
    :
    mFileName(fileName),
    mHeight(0),
    mWidth(0)
{
}

int XStxExampleClientFileInput::parseFile()
{
    if (mFileName.empty())
    {
        printf("\nNo file name provided");
        return FILE_INPUT_ERROR;
    }

    // Open file
    std::ifstream file;
    file.open(mFileName.c_str(), std::ios::in);
    if (file.fail())
    {
        printf("\n%s not found", mFileName.c_str());
        return FILE_INPUT_ERROR;
    }

    std::string line;

    // parse file
    while ( getline(file, line) )
    {
        if (line.length())
        {
            char first = line[0];
            if (first == '#')
            {
                // This line is a comment, ignore it
                continue;
            }
        }

        size_t pos = 0;
        std::string::size_type n = line.find("=");
        if (n == 0 || n == line.length()-1)
        {
            printf("\nInvalid input provided: %s", line.c_str());
            continue;
        }

        if (std::string::npos != n)
        {
            std::string lhs = line.substr(pos, n);
            std::transform(lhs.begin(), lhs.end(), lhs.begin(), ::toupper);
            std::string rhs = line.substr(n+1, line.length());

            mOptions[lhs] = rhs;
        }
    }

    return FILE_INPUT_SUCCESS;
}


int XStxExampleClientFileInput::getHeight(uint32_t& height)
{
    return getInt(inputVariables::height, height);
}

int XStxExampleClientFileInput::getWidth(uint32_t& width)
{
    return getInt(inputVariables::width, width);
}

int XStxExampleClientFileInput::getInt(const char* var, uint32_t& nVal)
{
    if (mOptions.size() == 0)
    {
        parseFile();
        if (mOptions.size() == 0)
        {
            return FILE_INPUT_ERROR;
        }
    }

    std::string val = mOptions[var];
    if (!val.empty())
    {
        nVal = atoi(val.c_str());
        return FILE_INPUT_SUCCESS;
    }

    return FILE_INPUT_ERROR;
}

XStxExampleClientFileInput::~XStxExampleClientFileInput(void)
{
}
