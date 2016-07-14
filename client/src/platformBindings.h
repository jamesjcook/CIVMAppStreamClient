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

/**
 * @file platformBindings.h
 *
 * Platform Specific Bindings.
 */

#ifndef _included_platformBindings_h
#define _included_platformBindings_h

#include <stdint.h>

/**
 * @ingroup MouseFlags Flags for the mouse.
 * @{
 */

/**
 * Mouse button 1 down.
 */
static const uint32_t CET_MOUSE_1_DOWN = 0x01;
/**
 * Mouse button 1 up.
 */
static const uint32_t CET_MOUSE_1_UP = 0x02;
/**
 * Mouse button 2 down.
 */
static const uint32_t CET_MOUSE_2_DOWN = 0x04;
/**
 * Mouse button 2 up.
 */
static const uint32_t CET_MOUSE_2_UP = 0x08;
/**
 * Mouse button 3 down.
 */
static const uint32_t CET_MOUSE_3_DOWN = 0x10;
/**
 * Mouse button 3 up.
 */
static const uint32_t CET_MOUSE_3_UP = 0x20;
/**
 * Mouse wheel message.
 */
static const uint32_t CET_MOUSE_WHEEL = 0x0400;
/**
 * Extra flag that indicates the mouse message came from a touch.
 */
static const uint32_t CET_TOUCH_FLAG = 0x8000;

/**
 *  @}
 */

/**
 * Request a new OpenGL/DirectX frame be rendered and presented.
 */
void platformNewFrame();

/**
 * Let the platform know we've connected.
 */
void platformOnConnectSuccess();

/**
 * Let the platform know we are reconnecting.
 *
 * @param[in] timeoutMs How long we try to reconnect.
 * @param[in] message message to display.
 */
void platformOnReconnecting(uint32_t timeoutMs, const char *message);

/**
 * Let the platform know we are reconnected.
 */
void platformOnReconnected();

/**
 * Display an error message to the user.
 *
 * @param[in] fatal   True if the error is fatal (the app should therefore
 *       restart).
 * @param[in] message Message to display.
 */
void platformErrorMessage(bool fatal, const char *message);

/**
 * For platforms that support hardware decoding directly to a texture,
 * bind the texture ID that we want to use. Must be called from an OpenGL
 * thread on OpenGL platforms.
 *
 * @param textureID Texture ID that will get hardware decoded images.
 *
 * @return The same texture ID passed in, or a new texture ID if a
 *                  different one is to be used.
 */
int platformBindVideoTexture(int textureID);

#endif

