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


#ifndef _included_XStxExampleClientDefines_h
#define _included_XStxExampleClientDefines_h

#include "stdint.h"

namespace clientDefaults
{

namespace standalone
{
    static const char * serverIP = "127.0.0.1";
    static const uint16_t port = 80;
    static const char * sessionId = "9070-0";
}
namespace entitlement
{
    /**
     * Enter your own values here
     */

    static const char * identityToken = "user@domain.com";
    static const char * applicationId = "87GSV8G7DFSTV87GF";
    static const char * serverUrl = "example.com/e, or 123.123.123.123";
    static const uint32_t port = 8080;
    static const bool shouldTerminatePrevious = true;
}

}

#endif
