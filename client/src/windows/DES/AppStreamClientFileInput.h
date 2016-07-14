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


#pragma once
#include <string>
#include <map>
#include <stdint.h>
#include "AppStreamClientHelpers.h"
namespace inputVariables 
{
    static const char * height = "HEIGHT";
    static const char * width = "WIDTH";
    static const char * identityToken = "USERNAME";
    static const char * appId = "APPID";
    static const char * entitlementServerUrl = "ENTITLEMENTSERVER";
    static const char * terminatePrevious = "TERMINATEPREVIOUS";
}
/**
 * Class parses the provided input file.  Each varible and value combination
 * will be separated by an '=' sign.  Each combination should be in a
 * separate line.
 *
 * VAR=VALUE
 *
 * Note that there are no spaces between them.
 *
 * The following are valid variable names
 *
 * HEIGHT - height of your render window (e.g. 720)
 * WIDTH - width of your render window (e.g. 1280)
 *
 * USERNAME - user name that will be passed to your DES
 * APPID - application id
 * ENTITLEMENTSERVER - url to DES
 * TERMINATEPREVIOUS - set "0" to *NOT* terminate previous session. Anything
 *                     else is treated as 1, default is 1 (true)
 */
class XStxExampleClientFileInput
{
public:

    XStxExampleClientFileInput(const char* fileName);
    ~XStxExampleClientFileInput();
    
    /**
     * returns an entitlement url based on contents in fileName
     * 
     * [out] entitlementUrl the entitlement url to connect to
     *
     * returns a 0 on success and -1 on failure.
     */
    int getEntitlementUrl(std::string& entitlementUrl);
    
    /**
     * returns provided HEIGHT value in input file
     *
     * returns -1 if height is not specified
     */
    int getHeight(uint32_t& height);
    
    /**
     * returns provided WIDTH value in input file
     *
     * returns -1 if width is not specified
     */
    int getWidth(uint32_t& width);

protected:
    
    int getInt(const char* var, uint32_t& nVal);
    int parseFile();
    int retrieveEntitlementUrl(std::string& entitlementUrl);

    uint32_t mHeight;
    uint32_t mWidth;

    std::string mFileName;
    std::map<std::string, std::string> mOptions;
};

class EntitlementInfoConfig : public XStxExampleClientFileInput
{
public:
    EntitlementInfoConfig() : XStxExampleClientFileInput("config.dat")
    {
    }

    void getEntitlementInfo(helpers::EntitlementInfo& entitlementInfo)
    {
        entitlementInfo.serverUrl = mOptions[inputVariables::entitlementServerUrl];
        entitlementInfo.identityToken = mOptions[inputVariables::identityToken];
        entitlementInfo.applicationId = mOptions[inputVariables::appId];
    }
};


