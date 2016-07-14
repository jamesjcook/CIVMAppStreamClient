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


/* this is move from RenderWindow class
    What happen before
    - RenderWindow has a DirectX window
    - DirecX Window has: entitlement Dialog, which uses the real code to talk to DES to get entitlement URL
    - RenderWindow then has these function below to pass the entitlement info to the DirectX window, then the entitlement dialog

*/









// WinEntitlementPromptDlg.h : header file
//

#pragma once
#define _WIN32_WINNT 0x0501
#include "../resource.h"
#include "AppStreamClientHelpers.h"
#include "../AppStreamClientDefines.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxstr.h>

#ifndef _AFX_NO_OLE_SUPPORT
    #include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
    #include <afxcmn.h>             // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxdhtml.h>        // HTML Dialogs

#include <string>
#include <stdint.h>

      /**
     * Set any pre-populated entitlement information
     */
    void setEntitlementInfo(const helpers::EntitlementInfo& entitlementInfo);
    
    /**
     * non-blocking.  Turns on a flag so render window knows it should
     * show entitlement prompt in its next cycle
     */
    void setShouldShowEntitlementPrompt(const char* errText);


// WinEntitlementPromptDlg dialog
class WinEntitlementPromptDlg : public CDHtmlDialog
{
// Construction
public:
    static INT_PTR showModalDialog(std::string& entitlementUrl,
        const char* serverUrl = clientDefaults::entitlement::serverUrl,
        const char* appId = clientDefaults::entitlement::applicationId,
        const char* identityToken = clientDefaults::entitlement::identityToken,
        const char* errorText = "");

    WinEntitlementPromptDlg(const char* serverUrl = clientDefaults::entitlement::serverUrl,
        const char* appId = clientDefaults::entitlement::applicationId,
        const char* identityToken = clientDefaults::entitlement::identityToken,
        const char* errText = "",
        CWnd* pParent = NULL );	// standard constructor

    const char* getEntitlementUrl();

    /* accessors */
    void setErrorText(const char* errorText);

protected:
    virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

    HRESULT OnButtonOK(IHTMLElement *pElement);
    HRESULT OnButtonCancel(IHTMLElement *pElement);

    virtual void OnOK();

    void showError(const char* error);

// Implementation
protected:
    HICON m_hIcon;

    CString mEntitlementServerUrl;
    CString mIdentityToken;
    CString mAppId;
    CString mPort;
    int mTerminatePreviousSession;
    CString mSessionId;
    CString mErrorText;
    CString mEntitlementUrl;

    // Generated message map functions
    virtual BOOL OnInitDialog();
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    DECLARE_MESSAGE_MAP()
    DECLARE_DHTML_EVENT_MAP()
};
