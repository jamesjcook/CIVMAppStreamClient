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


// WinEntitlementPromptDlg.cpp : implementation file
//

#include "WinEntitlementPromptDlg.h"
#include "afxdialogex.h"
#include "Util.h"
#include <winhttp.h>
#include <sstream>
#include "EntitlementRetriever.h"
#include "../AppStreamClientDefines.h"

namespace elementIds
{
    static const char * identityToken = "IdIdentityToken";
    static const char * applicationId = "IdApplicationId";
    static const char * entitlementServerUrl = "IdEntitlementServerUrl";
    static const char * entitlementPort = "IdEntitlementPort";
    static const char * errorMessage = "IdErrorMessage";
    static const char * terminatePreviousSession = "IdTerminatePreviousSession";
}

INT_PTR WinEntitlementPromptDlg::showModalDialog(std::string& entitlementUrl,
        const char* serverUrl,
        const char* appId,
        const char* identityToken,
        const char* errorText)
{
    std::string validServerUrl(serverUrl);
    if (validServerUrl.empty())
    {
        validServerUrl = clientDefaults::entitlement::serverUrl;
    }

    std::string validAppId(appId);
    if (validAppId.empty())
    {
        validAppId = clientDefaults::entitlement::applicationId;
    }

    std::string validIdentityToken(identityToken);
    if (validIdentityToken.empty())
    {
        validIdentityToken = clientDefaults::entitlement::identityToken;
    }

    INT_PTR retValue = IDCANCEL;

    CoInitializeEx(0, COINIT_APARTMENTTHREADED);

    HMODULE module = ::GetModuleHandle(NULL);

    if (AfxWinInit(module, NULL, ::GetCommandLine(), SW_SHOW))
    {  
        WinEntitlementPromptDlg dlg(validServerUrl.c_str(),
            validAppId.c_str(),
            validIdentityToken.c_str(),
            errorText);
        
        retValue = dlg.DoModal();

        if (retValue == IDOK)
        {
            entitlementUrl = dlg.getEntitlementUrl();
        }

    }

    CoUninitialize();

    return retValue;
}

// WinEntitlementPromptDlg dialog

BEGIN_DHTML_EVENT_MAP(WinEntitlementPromptDlg)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


WinEntitlementPromptDlg::WinEntitlementPromptDlg(const char* serverUrl,
        const char* appId,
        const char* identityToken,
        const char* errText,
        CWnd* pParent /* =NULL */)
    :
    mIdentityToken(identityToken),
    mAppId(appId),
    mEntitlementServerUrl(serverUrl),
    mErrorText(errText),
    CDHtmlDialog(IDD_ENTITLEMENT_DIALOG, IDH_ENTITLEMENT_HTML, pParent)
{
    mPort.FormatMessage("%d", clientDefaults::entitlement::port);
    mTerminatePreviousSession = clientDefaults::entitlement::shouldTerminatePrevious ? 1 : 0;
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void WinEntitlementPromptDlg::DoDataExchange(CDataExchange* pDX)
{
    CDHtmlDialog::DoDataExchange(pDX);
    DDX_DHtml_ElementValue(pDX, elementIds::identityToken, mIdentityToken);
    DDX_DHtml_ElementValue(pDX, elementIds::applicationId, mAppId);
    DDX_DHtml_ElementValue(pDX, elementIds::entitlementServerUrl, mEntitlementServerUrl);
    DDX_DHtml_ElementValue(pDX, elementIds::entitlementPort, mPort);
    DDX_DHtml_ElementInnerText(pDX, elementIds::errorMessage, mErrorText);
    DDX_DHtml_CheckBox(pDX, elementIds::terminatePreviousSession, mTerminatePreviousSession);
}

BEGIN_MESSAGE_MAP(WinEntitlementPromptDlg, CDHtmlDialog)
END_MESSAGE_MAP()


// WinEntitlementPromptDlg message handlers

BOOL WinEntitlementPromptDlg::OnInitDialog()
{
    CDHtmlDialog::OnInitDialog();

    // Set the icon for this dialog.  The framework does this automatically
    //  when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);			// Set big icon
    SetIcon(m_hIcon, FALSE);		// Set small icon
    return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void WinEntitlementPromptDlg::OnPaint()
{
    if (IsIconic())
    {
        CPaintDC dc(this); // device context for painting

        SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

        // Center icon in client rectangle
        int cxIcon = GetSystemMetrics(SM_CXICON);
        int cyIcon = GetSystemMetrics(SM_CYICON);
        CRect rect;
        GetClientRect(&rect);
        int x = (rect.Width() - cxIcon + 1) / 2;
        int y = (rect.Height() - cyIcon + 1) / 2;

        // Draw the icon
        dc.DrawIcon(x, y, m_hIcon);
    }
    else
    {
        CDHtmlDialog::OnPaint();
    }
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR WinEntitlementPromptDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

HRESULT WinEntitlementPromptDlg::OnButtonOK(IHTMLElement* /*pElement*/)
{
    UpdateData(TRUE);
    showError("");

    std::string identityToken = (LPCSTR)mIdentityToken;
    if (identityToken.empty())
    {
        showError("You must provide a User ID.");
        return S_FALSE;
    }

    std::string appId = (LPCSTR)mAppId;
    if (appId.empty())
    {
        showError("You must provide an Application ID.");
        return S_FALSE;
    }

    std::string entitlementServerUrl = (LPCSTR)mEntitlementServerUrl;
    if (entitlementServerUrl.empty())
    {
        showError("You must provide an Entitlement Service.");
        return S_FALSE;
    }

    // TODO: skip entitlement here if standalone is set?
    EntitlementRetriever ret(identityToken,
        appId,
        entitlementServerUrl,
        clientDefaults::entitlement::port,
        mTerminatePreviousSession > 0);

    std::string entitlementUrl;
    bool succeeded = ret.retrieveEntitlementUrl(entitlementUrl);

    if (!succeeded)
    {
        // If it fails, the output parameter is of EntitlementRetriever 
        // is the body of the DES response, which will give us an error string
        std::string errorText = entitlementUrl;

        if (errorText.empty())
        {
            uint32_t errorCode = ret.getLastError();
            std::stringstream ss;
            if (errorCode > WINHTTP_ERROR_BASE)
            {
                ss << "We are not able to connect to entitlement server. Please try again.";
            }
            else
            {
                ss << "Service responded with code";
            }
            
            ss << "[";
            ss << errorCode;
            ss << "]";
            errorText = ss.str();
        }

        showError(errorText.c_str());

        // In error handling, output this to user in UI
        printf("\n\nCouldn't get entitlement URL from server"
                "\n\nError response from server:\n%s\n", 
                entitlementUrl.c_str());

        UpdateData(TRUE); // refresh UI with error text

        return S_FALSE; // Return here, so that dialog remains open
    }
    else
    {
        mEntitlementUrl = entitlementUrl.c_str();
    }

    EndDialog(IDOK); // only exit UI when everything succeeds
    return S_OK;
}

HRESULT WinEntitlementPromptDlg::OnButtonCancel(IHTMLElement* /*pElement*/)
{
    OnCancel();
    return S_OK;
}

// Called by base class when return key is hit
void WinEntitlementPromptDlg::OnOK()
{
    // Manually trigger "connect" button handler
    OnButtonOK(0);
    return;
}

void WinEntitlementPromptDlg::showError(const char* error)
{
    std::string tmp(error);
    std::wstring errorText = std::wstring(tmp.begin(), tmp.end());
    SetElementText(elementIds::errorMessage, (BSTR)errorText.c_str());
}

const char* WinEntitlementPromptDlg::getEntitlementUrl()
{
    return (LPCSTR)mEntitlementUrl;
}

void WinEntitlementPromptDlg::setErrorText(const char* errorText)
{
    mErrorText = errorText;
}
