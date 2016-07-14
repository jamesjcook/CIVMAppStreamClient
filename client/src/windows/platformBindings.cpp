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

#include "DirectXRenderer.h"
#include "ClientWindow.h"
#include "platformBindings.h"

#undef LOG_TAG
#define LOG_TAG "platformBindings"
#include "log.h"

void platformNewFrame()
{
}

//this is a global variable defined in AppStreamClient.h and initalized in AppStreamClient.cpp
extern ClientWindow *gWindow;
extern DirectXRenderer *gDirectXRenderer;
void platformErrorMessage(bool fatal, const char *message)
{
    if (!fatal)
    {
        gWindow->setShouldShowEntitlementPrompt(message);
    }

    gDirectXRenderer->setTextBoxText("status",message);
    gDirectXRenderer->showTextBox("status");
}

/**
 * Let the platform know we've connected.
 */
void platformOnConnectSuccess()
{
}

/**
 * Let the platform know we are reconnecting.
 */
void platformOnReconnecting(uint32_t timeoutMs, const char *message)
{
    gDirectXRenderer->setTextBoxText("status",message);
    gDirectXRenderer->showTextBox("status");
    gDirectXRenderer->setTextBoxText("reconnect", "Trying to reconnect");
    gDirectXRenderer->showTimedTextBox("reconnect", timeoutMs);
}

/**
 * Let the platform know we are reconnected.
 */
void platformOnReconnected()
{
    gDirectXRenderer->hideTextBox("status");
    gDirectXRenderer->hideTextBox("reconnect");
}


