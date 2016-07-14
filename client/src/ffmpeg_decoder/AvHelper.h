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



#ifndef _included_AvHelper_h
#define _included_AvHelper_h


/**
 * A class to coordinate working with FFmpeg. Specifically, it wraps the
 * logging output.
 */
class AvHelper
{

public:

    static void initialize();

    static void terminate();
};

#endif // _included_AvHelper_h


