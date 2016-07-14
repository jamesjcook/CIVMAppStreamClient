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


#include <stdio.h>
#include <string>

#include "XStx/common/XStxAPI.h"
#include "XStx/client/XStxClientAPI.h"

#include "ClientWindow.h"
#include "DirectXRenderer.h"
#include "AppStreamWrapper.h"
#include "InputHandling/WindowInputCapture.h"
#include "AppStreamClient.h"
#include "DES/AppStreamClientHelpers.h"
#include "DES/AppStreamClientFileInput.h"

static void printUsage()
{/*
    printf(
        "\nXStxExampleClient -u <entitlementUrl> -w <videoWindowWidth> -h <videoWindowHeight> -s <serverIp> -p <serverPort> -i <sessionId>\n"
        "\t videoWindowWidth defaults to 1280.\n"
        "\t videoWindowHeight defaults to 720.\n"
        "\t --- The following arguments only work with the stand-alone (dummy) SessionMonger ---\n"
        "\t serverIp defaults to %s.\n"
        "\t serverPort defaults to %u.\n"
        "\t sessionId defaults to %s.\n"
        "\t with the stand-alone Monger entitlementUrl can also be of the form:\n"
        "\t\t <serverIp>:<port>?sessionId=<sessionId>\n",
        clientDefaults::standalone::serverIP,
        clientDefaults::standalone::port,
        clientDefaults::standalone::sessionId);
        */
}

bool parseCommandLine(int argc, char ** argv,
                      std::string& entitlementUrl,
                      std::string& serverName,
                      std::string& sessionId,
                      uint32_t& port,
                      uint32_t& width,
                      uint32_t& height)
{
    bool skipEntitlement = false;

    int count = 1;
    while (count < argc)
    {
        if (strcmp(argv[count], "-u") == 0)
        {
            count++;

            if (count < argc) {
                entitlementUrl = argv[count];
                count++;
            }
            skipEntitlement = true;
        }
        // server IP address
        else if (strcmp(argv[count], "-s") == 0)
        {
            count++;

            if (count < argc) {
                serverName = argv[count];
                count++;
            }
            skipEntitlement = true;

        }
        // port number
        else if (strcmp(argv[count], "-p") == 0)
        {
            count++;

            if (count < argc) {
                port = atoi(argv[count]);
                count++;
            }
            skipEntitlement = true;
        }
        // width
        else if (strcmp(argv[count], "-w") == 0)
        {
            count++;

            if (count < argc) {
                width = atoi(argv[count]);
                count++;
            }
        }
        // height
        else if (strcmp(argv[count], "-h") == 0)
        {
            count++;

            if (count < argc) {
                height = atoi(argv[count]);
                count++;
            }
        }
        // session ID
        else if (strcmp(argv[count], "-i") == 0)
        {
            count++;

            if (count < argc) {
                sessionId = argv[count];
                count++;
            }
            skipEntitlement = true;
        }
        else
        {
            printf("Unexpected command line argument: \"%s\"\n", argv[count]);
            printUsage();
            exit(1);
        }
    }

    return skipEntitlement;
}


int main(int argc, char ** argv)
{

    AppStreamWrapper *appStreamWrapper;
    //TODO: fix hardcode here
    std::string serverName = "127.0.0.1";//clientDefaults::standalone::serverIP;
    uint32_t port = 80;//clientDefaults::standalone::port;
    std::string sessionId = "9070-0";//clientDefaults::standalone::sessionId;

    uint32_t width = 1280;
    uint32_t height = 720;
    //bool terminateAnyPreviousSession =  clientDefaults::entitlement::shouldTerminatePrevious;


    std::string entitlementUrl;
    helpers::EntitlementInfo entitlementInfo;

    bool skipEntitlement = false;   // If entitlement url, streaming server name,
                                    // or streaming server port are specified
                                    // at the command line, the exchange
                                    // with the entitlement server is skipped.

    skipEntitlement = parseCommandLine(
        argc,
        argv,
        entitlementUrl,
        serverName,
        sessionId,
        port,
        width,
        height);

    if (entitlementUrl.empty() && skipEntitlement)
    {
        char buf[1024];
        snprintf(
            buf,
            sizeof(buf),
            "ssm://%s:%u?sessionId=%s",
            serverName.c_str(),
            port,
            sessionId.c_str());
        entitlementUrl = buf;
    }

    if (entitlementUrl.empty())
    {
        EntitlementInfoConfig fileInput;
        int ret = fileInput.getEntitlementUrl(entitlementUrl);

        if (ret < 0)
        {
            printf("\nInput file failed to get entitlement : %s\n",
                entitlementUrl.c_str());
            entitlementUrl = "";
        }

        uint32_t newHeight = height;
        ret = fileInput.getHeight(newHeight);
        if (ret == 0)
        {
            height=newHeight;
        }

        uint32_t newWidth = width;
        ret = fileInput.getWidth(newWidth);
        if (ret == 0)
        {
            width=newWidth;
        }

        // get any entitlement info that was passed in
        fileInput.getEntitlementInfo(entitlementInfo);
    }


    // try get the version just to exercise the API, and demonstrate its possible
    XStxLibraryVersion libVersion;
    XStxGetLibraryVersion(sizeof(libVersion), &libVersion);
    assert(libVersion.mMajorVersion == XSTX_CLIENT_API_VERSION_MAJOR);
    printf(
        "%s (%d.%d.%d)\n",
        libVersion.mDescription,
        libVersion.mMajorVersion,
        libVersion.mMinorVersion,
        libVersion.mBuild);

    gDirectXRenderer = new DirectXRenderer();
    gWindow = new ClientWindow();

    gWindow->setEntitlementUrl(entitlementUrl.c_str());
    gWindow->setEntitlementInfo(entitlementInfo);
    gWindow->setFullscreen(false);
    gWindow->setDimensions(width,height);
    gWindow->init();

    gDirectXRenderer->setWindowHandle(gWindow->getWindowHandle());

    appStreamWrapper = new AppStreamWrapper();
    appStreamWrapper->init();

    gInputHandler = new WindowInputCapture(gWindow->getWindowHandle(), appStreamWrapper, gWindow);
    gInputHandler->setWindowBorderSize(gWindow->getBorderWidth(), gWindow->getBorderHeight());
    gWindow->setAppStreamWrapper(appStreamWrapper);

    gWindow->beginDisplay();

    // clean up

	appStreamWrapper->stop();
    appStreamWrapper->recycle();

	//clean up global variable;

    gWindow->stopDrawing();


	delete gInputHandler;
	delete gWindow;

    //We should NOT delete the platform-specific video renderer here since
    //AppStreamWrapper object will take care of deleting the object
    //Most platforms do not need to keep track of the video renderer and left
    //the handling of life cycle of the video renderer to AppStreamWrapper, Windows should
    //follow the same style.
	//delete gDirectXRenderer;

	delete appStreamWrapper;

    return 0;
}
