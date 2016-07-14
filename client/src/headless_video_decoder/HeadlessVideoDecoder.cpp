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


#include "MUD/threading/ThreadUtil.h"
#include "HeadlessVideoDecoder.h"

#undef LOG_TAG
#define LOG_TAG "HeadlessVideoDecoder"

#include "log.h"

/** Constructor */
HeadlessVideoDecoder::HeadlessVideoDecoder()
{
}

/** Destructor */
HeadlessVideoDecoder::~HeadlessVideoDecoder()
{
}

/** initialize */
XStxResult HeadlessVideoDecoder::init()
{
    // successfully initialized
    return XSTX_RESULT_OK;
}

/**
 * Decode frame
 * @param[in] enc frame to be decoded
 * @param[out] dec holder for decoded frame
 */
XStxResult HeadlessVideoDecoder::decodeFrame(
    XStxEncodedVideoFrame *enc,
    XStxRawVideoFrame *dec)
{
    if (enc == NULL || dec == NULL)
    {
        return XSTX_RESULT_INVALID_ARGUMENTS;
    }

    mud::ThreadUtil::sleep(5);
    dec->mTimestampUs = enc->mTimestampUs;

    // successfully decoded frame
    return XSTX_RESULT_OK;
}

