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


#ifndef _included_ExampleClientHelpers_h
#define _included_ExampleClientHelpers_h
#include <string>
#include "XStx/common/XStxStopReasonAPI.h"

namespace helpers
{
    // Simple object for passing around entitlement info
    struct EntitlementInfo
    {
        EntitlementInfo()
        {
        }

        EntitlementInfo(const std::string& entitlementServerUrl,
            const std::string& appId,
            const std::string& identity)
            :
            serverUrl(entitlementServerUrl),
            applicationId(appId),
            identityToken(identity)
        {
        }

        std::string serverUrl;
        std::string applicationId;
        std::string identityToken;
    };

    static void getStopReasonMessage(XStxStopReason stopReason, std::string& msg)
    {
        std::string errMsg = "";

        switch(stopReason)
        {
        case XSTX_STOP_REASON_REQUESTED:
            errMsg = "Requested";
            break;
        case XSTX_STOP_REASON_CONNECT_TIMED_OUT:
        case XSTX_STOP_REASON_TCP_ACCEPT_SOCKET:
        case XSTX_STOP_REASON_TCP_BIND_SOCKET:
        case XSTX_STOP_REASON_TCP_CONNECT_FAILED:
        case XSTX_STOP_REASON_TCP_CREATE_SOCKET:
        case XSTX_STOP_REASON_TCP_LISTEN_SOCKET:
        case XSTX_STOP_REASON_TCP_UNKNOWN_HOST:
        case XSTX_STOP_REASON_INVALID_IP:
            errMsg = "We are not able to connect to the server. Please try again.";
            break;
        case XSTX_STOP_REASON_ENCODER_CONFIG_ERROR:
        case XSTX_STOP_REASON_SESSION_CLOSED:
        case XSTX_STOP_REASON_INIT_PROBLEM:
        case XSTX_STOP_REASON_UNKNOWN_NETWORK:
            errMsg = "Something went wrong.";
            break;
        case XSTX_STOP_REASON_CONNECTION_LOST:
            errMsg = "We lost the connection to the server. Please try again.";
            break;
        case XSTX_STOP_REASON_SESSION_REQUEST_FAILED:
            errMsg = "Your account was recognized, but there was a problem connecting to the application.";
            break;
        case XSTX_STOP_REASON_PROTOCOL_INCOMPATIBLE:
            errMsg = "The client application must be upgraded to connect to the server. Please upgrade here and try again.";
            break;
        case XSTX_STOP_REASON_SESSION_REQUEST_INVALID_ENTITLEMENT_URL:
            errMsg = "The link to the streaming server has expired.";
            break;
        case XSTX_STOP_REASON_SESSION_REQUEST_NO_AVAILABLE_INSTANCE:
            errMsg = "All of our streaming servers are currently in use. Please try again in a few minutes.";
            break;
        case XSTX_STOP_REASON_UNKNOWN:
        default:
            errMsg = "Unknown Error.";
            break;
        }
        msg = errMsg;
    }
}

#endif
