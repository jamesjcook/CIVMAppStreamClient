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


#include "EntitlementRetriever.h"

#include <Windows.h>
#include <Winhttp.h>
#include <Strsafe.h>
#include <new>
#include <iostream>
EntitlementRetriever::EntitlementRetriever(const std::string& identityToken,
					   const std::string& applicationId,
					   const std::string& entitlementServerUrl,
					   uint16_t entitlementPort,
					   bool terminateExistingSession) :
  mIdentityToken(identityToken),
  mApplicationId(applicationId),
  mEntitlementServerUrl(entitlementServerUrl),
  mEntitlementPort(entitlementPort),
  mTerminateExistingSession(terminateExistingSession)
{
}
 
// local methods

BOOL addRequestHeaders(HINTERNET hRequest, const std::string& identityToken, uint32_t& errorCode);
BOOL receiveResponse(HINTERNET hRequest, uint32_t& responseCode, 
                     std::string& entitlementUrl, uint32_t& errorCode);
BOOL parseResponseHeaders(HINTERNET hRequest, uint32_t& responseCode, uint32_t& errorCode);
BOOL getResponseBody(HINTERNET hRequest, std::string& entitlementUrl, uint32_t& errorCode);
HINTERNET sendRequest(HINTERNET hConnect,
                      const std::string& IdentityToken,
                      const std::string& applicationId,
                      bool terminateExistingSession,
                      uint32_t& errorCode);

bool EntitlementRetriever::retrieveEntitlementUrl(std::string& responseBody)
{        
    HINTERNET hSession = NULL, 
              hConnect = NULL,
              hRequest = NULL;
    bool result = FALSE;
    uint32_t responseCode = 0;
    std::wstring wstrUrl;

    responseBody.clear();

    // Use WinHttpOpen to obtain an HINTERNET handle.
    hSession = WinHttpOpen(
        L"XStxExampleClient/EntitlementRetrieverWindows", 
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, 
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (NULL == hSession)
    {
        lastError = GetLastError();
        printf("Error in WinHttpOpen. Code is %d\n", lastError);
        goto cleanup;
    }

    // Specify an HTTP server.
    wstrUrl = std::wstring(mEntitlementServerUrl.begin(), mEntitlementServerUrl.end());        
    hConnect = WinHttpConnect(
        hSession, wstrUrl.c_str(),mEntitlementPort, 0);
    if(NULL == hConnect)
    {
        lastError = GetLastError();
        printf("Error in WinHttpConnect. Code is %d\n", lastError);      
        if(12005 == lastError)
        {
            printf("\tThe URL is not valid: %s\n", mEntitlementServerUrl.c_str());
        }
        goto cleanup;
    }

    hRequest = sendRequest(hConnect, mIdentityToken, 
                    mApplicationId, mTerminateExistingSession, lastError);
    if(NULL == hRequest)
    {
        goto cleanup;
    }
   
    if(FALSE == receiveResponse(hRequest, responseCode, responseBody, lastError))
    {
        goto cleanup;
    }

    if(201 == responseCode) 
    {
        result = true;
    } 
    else 
    {
        lastError = responseCode;
        printf("\n--->EntitlementRetriever received response code: %u\n",
            responseCode);
    }

cleanup: 
    if(NULL != hRequest)
    {

        WinHttpCloseHandle(hRequest);
    }
    if(NULL != hConnect)
    {
        WinHttpCloseHandle(hConnect);
    }
    if(NULL != hSession)
    {
        WinHttpCloseHandle(hSession);
    }
    
    std::cout<<"\n======entitlement URL is: "<< responseBody<<"\n";
    return result;
}   

uint32_t EntitlementRetriever::getLastError()
{
    return lastError;
}

HINTERNET sendRequest(HINTERNET hConnect,
                      const std::string& identityToken,
                      const std::string& applicationId,
                      bool terminateExistingSession,
                      uint32_t& errorCode)
{
    HINTERNET hRequest = NULL;
    BOOL bResults = FALSE;

    std::wstring resource(L"/api/entitlements/" + 
    std::wstring(applicationId.begin(), applicationId.end())); 
    hRequest = WinHttpOpenRequest( 
        hConnect, L"POST", resource.c_str(),
        NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    // Send a request.
    if (hRequest)
    {
        bResults = addRequestHeaders(hRequest, identityToken, errorCode);
        if(FALSE == bResults)
        {
            WinHttpCloseHandle(hRequest);
            return NULL;
        }
    }

    // create and send request
    std::string body(std::string("terminatePrevious=") + 
                (terminateExistingSession ? "true" : "false"));      
            
    bResults = WinHttpSendRequest(
                hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, 
                (LPVOID)body.c_str(), (DWORD)body.size(), 
                (DWORD)body.size(), 0);

    if(FALSE == bResults)
    {
        errorCode = GetLastError();
        printf("Error: WinHttpSendRequest returns false with code %d\n",
             errorCode);
        if(errorCode == 12007)
        {
            printf("\tServer name cannot be resolved\n");
        }

        WinHttpCloseHandle(hRequest);
        return NULL;
    }
    return hRequest; 
}


BOOL addRequestHeaders(HINTERNET hRequest,
                       const std::string& identityToken,
                       uint32_t& errorCode)
{
    BOOL bResults;
    std::wstring formHeader(
            L"Content-Type:application/x-www-form-urlencoded");
    bResults = WinHttpAddRequestHeaders(
                    hRequest, formHeader.c_str(), 
                    -1L, 
                    WINHTTP_ADDREQ_FLAG_ADD);
    std::wstring authHeader(
        L"Authorization:Username" + 
        std::wstring().assign(identityToken.begin(),
        identityToken.end()) + L"\r\n");
    bResults = WinHttpAddRequestHeaders( 
        hRequest, authHeader.c_str(), 
        -1L, 
         WINHTTP_ADDREQ_FLAG_ADD
    );
    if(FALSE == bResults)
    {
        errorCode = GetLastError();
        printf("Error: WinHttpAddRequestHeaders returns false with code %d\n",
            errorCode);   
    }

    return bResults;    
}


BOOL receiveResponse(HINTERNET hRequest, 
                     uint32_t& responseCode, 
                     std::string& responseBody,
                     uint32_t& errorCode)
{
    BOOL bResults;
    
    bResults = WinHttpReceiveResponse( hRequest, NULL);
    if(FALSE == bResults)
    {
        return FALSE;
    }

    if(parseResponseHeaders(hRequest, responseCode, errorCode) == FALSE)
    {
        return FALSE;
    }
    
    if(getResponseBody(hRequest, responseBody, errorCode) == FALSE)
    {
        return FALSE;
    }
    return TRUE;
}

BOOL parseResponseHeaders(HINTERNET hRequest, 
                          uint32_t& returnCode, 
                          uint32_t& errorCode)
{
    LPVOID lpOutBuffer = NULL;
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    returnCode = 0;
    WinHttpQueryHeaders( hRequest, WINHTTP_QUERY_STATUS_CODE,
                         WINHTTP_HEADER_NAME_BY_INDEX, NULL,
                         &dwSize, WINHTTP_NO_HEADER_INDEX);

    // Allocate memory for the buffer.
    errorCode = GetLastError();
    if( errorCode != ERROR_INSUFFICIENT_BUFFER )
    {
        printf("Error: unexpected error code from WinHttpQueryHeaders: %d\n",
            errorCode);
            return FALSE;
    }

    lpOutBuffer = new (std::nothrow) WCHAR[dwSize/sizeof(WCHAR)];
    if(NULL == lpOutBuffer)
    {
        printf("Error: out of memory in parseReponseHeaders\n");
        return FALSE;
    }

    // Now, use WinHttpQueryHeaders to retrieve the header.
     bResults = WinHttpQueryHeaders(
         hRequest, WINHTTP_QUERY_STATUS_CODE, WINHTTP_HEADER_NAME_BY_INDEX,
         lpOutBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX);
     
     if(FALSE == bResults)
     {
        errorCode = GetLastError();
        printf("Error: WinHttpQueryHeaders failed with code %d\n",
              errorCode);
        delete lpOutBuffer;
        return FALSE;
     }
        
     std::wstring resultCode((wchar_t*)lpOutBuffer);
     returnCode = atoi(std::string(resultCode.begin(), resultCode.end()).c_str());     
   
     delete lpOutBuffer;
     return true;
}

BOOL getResponseBody(HINTERNET hRequest, 
                     std::string& responseBody, 
                     uint32_t& errorCode)
{
    BOOL bResults = FALSE;
    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;
   
    responseBody.clear();
    // Verify available data.
    do 
    {
        dwSize = 0;
        dwDownloaded = 0;
                       
        if(!WinHttpQueryDataAvailable( hRequest, &dwSize))
        {
            errorCode = GetLastError();
            printf( "Error %u in WinHttpQueryDataAvailable.\n",
                        errorCode);
            break;
        }
        if(dwSize > 0)
        {
            // Allocate space for the buffer.
            LPSTR pszOutBuffer = new char[dwSize+1];
            if (!pszOutBuffer)
            {
                printf("Error: getResponseBody: Out of memory\n");
                dwSize=0;
            }
            else
            {
                // Read the Data.
                ZeroMemory(pszOutBuffer, dwSize+1);
                if (!WinHttpReadData( hRequest, (LPVOID)pszOutBuffer, 
                                        dwSize, &dwDownloaded))
                {
                    errorCode = GetLastError();
                    printf( "Error %u in WinHttpReadData.\n", errorCode);
                }
                else 
                {
                    responseBody = std::string(pszOutBuffer);
                    bResults = TRUE;
                }            
                // Free the memory allocated to the buffer.
                delete [] pszOutBuffer;
            }
        }
    } while (dwSize > 0);
    
    return bResults;
}
