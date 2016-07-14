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


#ifndef _included_EntitlementRetriever_h
#define _included_EntitlementRetriever_h

#include <string>
#include <stdint.h>

class EntitlementRetriever
{
public:
    EntitlementRetriever(
        const std::string& identityToken,
		const std::string& applicationId,
		const std::string& entitlementServerUrl,
        uint16_t entitlementPort,
		bool terminateExistingSession);
  
    bool retrieveEntitlementUrl(std::string& entitlementUrl);
    uint32_t getLastError();

 private:
    const std::string& mIdentityToken;
    const std::string& mApplicationId;
    const std::string& mEntitlementServerUrl;
    uint16_t mEntitlementPort;
    bool mTerminateExistingSession;
    uint32_t lastError;
};



#endif
