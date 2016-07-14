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
#include "DES/EntitlementRetriever.h"
#include "Util.h"
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

int XStxExampleClientFileInput::getEntitlementUrl(std::string& entitlementUrl)
{
    if (mOptions.size() == 0)
    {
        int parseRet = parseFile();
        if (mOptions.size() == 0 || FILE_INPUT_ERROR == parseRet)
        {
            return FILE_INPUT_ERROR;
        }
    }

    return retrieveEntitlementUrl(entitlementUrl);
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

int XStxExampleClientFileInput::retrieveEntitlementUrl(std::string& entitlementUrl)
{
    std::string identityToken = mOptions[inputVariables::identityToken];
    std::string appId = mOptions[inputVariables::appId];
    std::string entitlementServerUrl = mOptions[inputVariables::entitlementServerUrl];
    std::string terminatePrevious = mOptions[inputVariables::terminatePrevious];

    // Mandatory inputs
    if (identityToken.empty() || 
        appId.empty() || 
        entitlementServerUrl.empty())
    {
        return FILE_INPUT_ERROR;
    }

    uint32_t nPort = clientDefaults::entitlement::port;

    bool terminate = clientDefaults::entitlement::shouldTerminatePrevious;
    if (!terminatePrevious.empty())
    {
        // if set to 0, then terminate previous is off
        terminate = !(terminatePrevious.compare("0") == 0);
    }

    EntitlementRetriever ret(identityToken,
        appId,
        entitlementServerUrl,
        nPort,
        terminate);

    bool succeeded = ret.retrieveEntitlementUrl(entitlementUrl);

    return succeeded ? FILE_INPUT_SUCCESS : FILE_INPUT_ERROR;
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
