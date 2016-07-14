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

#include "platformBindings.h"

#undef LOG_TAG
#define LOG_TAG "platformBindings"
#include "log.h"

void platformNewFrame()
{
}

void platformErrorMessage(bool fatal, const char *message)
{
    if (!fatal)
    {
        printf("\nFatal error: %s", message);
    }

    printf("\nError: %s", message);
}

/**
 * Let the platform know we've connected.
 */
void platformOnConnectSuccess()
{
}

