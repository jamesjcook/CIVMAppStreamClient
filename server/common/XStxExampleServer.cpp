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

#include <stdio.h>
#include <stddef.h>
#include <cassert>

#include "XStx/server/XStxServerAPI.h"
#include "ServerManagerListener.h"

static const char* TAG = "XStxExampleServer";

// If the symbol APPSTREAM_GAME is defined, we will run the game using AppStream
// This means apart from local output, the game will capture its output
// and supply it to AppStream to send to any connected clients
#if APPSTREAM_GAME

int runAsAppStreamGame(int argc, const char* argv[])
{
    /** Initialize for eventual clean-up */

    XStxServerLibraryHandle serverLibraryHandle = NULL;
    XStxServerManagerHandle serverManagerHandle = NULL;
    ServerManagerListener* serverManagerListener = NULL;

    // try get the version just to exercise the API, and demonstrate its possible
    XStxLibraryVersion libVersion;
    XStxGetLibraryVersion(sizeof(libVersion), &libVersion);
    assert(libVersion.mMajorVersion == XSTX_SERVER_API_VERSION_MAJOR);
    printf(
        "%s (%d.%d.%d)\n",
        libVersion.mDescription,
        libVersion.mMajorVersion,
        libVersion.mMinorVersion,
        libVersion.mBuild);

    /** Initialize the XStxServer library */

    XStxResult result = XStxServerLibraryCreate(
        XSTX_SERVER_API_VERSION_MAJOR,
        XSTX_SERVER_API_VERSION_MINOR,
        &serverLibraryHandle);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /**
     * Create a session manager instance (which in this case just fakes
     * receiving session requests from external services).
     */

    result = XStxServerLibraryCreateXStxServerManager(
        serverLibraryHandle,
        &serverManagerHandle);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /**
     * Create the application-specific server manager listener
     * that will start our hosted application and connect it to
     * an XStx server.
     */

    result = ServerManagerListener::createServerManagerListener(
        serverLibraryHandle,
        serverManagerListener);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /* Point the session manager at our listener */

    result = XStxServerManagerSetListener(
        serverManagerHandle,
        serverManagerListener->getServerManagerListener());

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /** Start the session manager */

    result = XStxServerManagerStart(serverManagerHandle);

    if (result != XSTX_RESULT_OK)
    {
        goto exit;
    }

    /** Wait for the session manager to exit */

    result = XStxServerManagerWait(serverManagerHandle);

exit:
    XStxResult cleanUpResult = XStxServerManagerRecycle(serverManagerHandle);
    if (cleanUpResult != XSTX_RESULT_OK) {
        printf("Failed to recycle ServerManager\n");
    }

    cleanUpResult = XStxServerLibraryRecycle(serverLibraryHandle);
    if (cleanUpResult != XSTX_RESULT_OK) {
        printf("Failed to recycle ServerLibrary\n");
    }

    delete serverManagerListener;

    printf("Exit! %s\n", XStxResultGetName(result));
    fflush(stdout);
    
    return (result != XSTX_RESULT_OK) ? -1 : 0;
}

#else
// Otherwise we run locally only and don't use AppStream at all

// Implement this method to run the application locally without using AppStream
// Note that if this method is not implemented and APPSTREAM_GAME is not defined,
// you will receive a linker error about an unresolved symbol 
// int __cdecl runLocal(int,char const * * const)

int runLocal(int argc, const char* argv[]); // forward declaration

#endif

/**
 * Main entry point for XStxExampleServer.
 *
 * By using XStxServerLibrarySetStandalone() this code drives a
 * single session with a canned session ID and application context.
 */

int main(int argc, const char* argv[])
{
#if APPSTREAM_GAME
    return runAsAppStreamGame(argc, argv);
#else
    return runLocal(argc, argv);
#endif
}
