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

#include "../OsUtils.h"

#include "TargetConditionals.h"


#import <Foundation/Foundation.h>
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#endif


#include <iostream>
#include <sstream>

#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>


using namespace mud;
using namespace std;

std::string OsUtils::mDynamicLibraryPath = "";

bool OsUtils::createDirectory(std::string dir) {
    return false; // not implemented
}

bool OsUtils::setEnv(const char * name, const char * value)
{
    return false; // not implemented
}

bool OsUtils::getOsDetails(string& OsName, int& majorVersion, int& minorVersion,
        int& buildNumber, string& details)
{

    majorVersion = -2;
    minorVersion = -2;
    buildNumber = -2;
    details.clear();

    @autoreleasepool {

        NSString *sysversion;

#if TARGET_OS_IPHONE
        OsName = "iOS";
        sysversion = [UIDevice currentDevice].systemVersion;
#else 
        OsName = "OSX";

        NSDictionary * versionDict = [NSDictionary dictionaryWithContentsOfFile:@"/System/Library/CoreServices/SystemVersion.plist"];
        sysversion = [versionDict objectForKey:@"ProductVersion"];
#endif

        NSArray *components = [sysversion componentsSeparatedByString:@"."];

        if(components.count > 0)
        {
            majorVersion = atoi([[components objectAtIndex:0] UTF8String] );
        }
        if(components.count > 1)
        {
            minorVersion = atoi([[components objectAtIndex:1] UTF8String]);
        }
        if(components.count > 2)
        {
            buildNumber = atoi([[components objectAtIndex:2] UTF8String]);
        }

        utsname sysInfo;
        if(uname(&sysInfo) == 0)
        {
            details = "{\"details\":\"" + string(sysInfo.version) + "\"}";
        }

    } // autorelease pool

    return true;
}


bool OsUtils::getHwInfo(string& hwInfo)
{

    static const char *QQ = "\"";

    stringstream ss;

    hwInfo.clear(); 

    // modern compiler complains about initing
    // non-const char with string literal
    // so creating a more compatible struct here
    struct hwctl { const char	*ctl_name; int	ctl_type; };

    // get listing of available info from sysctl.h
    struct hwctl hw_names [] = CTL_HW_NAMES;

#if  TARGET_OS_IPHONE
    ss << "{" << QQ << "iOSHwInfo" << QQ <<  ":{";
#else
    ss << "{" << QQ << "OSXHwInfo" << QQ <<  ":{";
#endif 


    for (int i=1;i<HW_MAXID;i++)
    {


        struct hwctl ctl = hw_names[i];
        char hwname[strlen(ctl.ctl_name)];
        sprintf(hwname,"hw.%s", ctl.ctl_name);
        size_t buflen;

        sysctlbyname(hwname, NULL, &buflen, NULL, 0);

        ss << QQ << ctl.ctl_name << QQ << ":";

        if( ctl.ctl_type == CTLTYPE_STRING)
        {
            char *buffer = (char*) malloc(buflen);
            sysctlbyname(hwname, buffer, &buflen, NULL, 0);
            ss << QQ << buffer << QQ;
            free(buffer);
        }
        else if (ctl.ctl_type == CTLTYPE_INT)
        {
            int result;
            sysctlbyname(hwname, &result, &buflen, NULL, 0);
            ss << result;
        }
        else if (ctl.ctl_type == CTLTYPE_QUAD )
        {
            uint64_t bigresult;
            sysctlbyname(hwname, &bigresult, &buflen, NULL, 0);
            ss << bigresult;
        }
        else
        {
            ss << QQ << "complex" << QQ;
        }

        if( i < (HW_MAXID-1)) {
            ss << ",";
        } else
        {
            ss << "}}";
            hwInfo = ss.str();
        }
    }

    return true;
}




