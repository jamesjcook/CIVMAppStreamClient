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


#ifndef APPSTREAM_CLIENT_H
#define APPSTREAM_CLIENT_H

class DirectXRenderer;
class WindowInputCapture;
class ClientWindow;


ClientWindow *gWindow;
//global variable
//this needs to be global because it will be accessed from AppStreamClient.cpp and videoPipeline.cpp
DirectXRenderer *gDirectXRenderer;
//this needs to be global because it will be access by a global function called WindowsProcessMessage in
//ClienWindow.cpp. This function need to be global(Windows style)
WindowInputCapture * gInputHandler;

 #endif
