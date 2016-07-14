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


#include "BitStreamReader.h"

BitStreamReader::BitStreamReader(uint8_t *streamData, size_t streamLength):
mCurrLoc(0),
mBitsRemaining(0),
mCurrBits(0),
mStreamData(streamData),
mStreamLength(streamLength)
{
}

uint32_t BitStreamReader::getBits(uint8_t numBits)
{
    if (numBits > 32) {
        printf("Cannot read that many bits at a time: %i\n", numBits);
        return 0;
    }

    if (numBits > totalBitsRemaining()) {
        printf("Not enough bits remaining in the stream. Remaining: %zu Requested: %i\n", totalBitsRemaining(), numBits);
        return 0;
    }

    //Shortcut if they aren't asking for any bits
    // (this happens frequently with the exp golomb encoding)
    if (numBits <= 0) {
        return 0;
    }

    //Make sure our bit buffer has at least as many bits as they want to read
    while (mBitsRemaining < numBits) {
        //Shift the current bits left
        mCurrBits = mCurrBits << 8;
        //Add the first byte in the stream
        mCurrBits += mStreamData[mCurrLoc++];
        //Update the bits remaining
        mBitsRemaining += 8;
    }

    //The bit buffer now holds enough bits for what they are trying to read

    //Shift to the right to drop off any extra bits we don't want
    uint8_t numBitsToDropOnRight = mBitsRemaining - numBits;
    uint32_t retValue = (uint32_t)(mCurrBits >> numBitsToDropOnRight);

    //Mask out any extra bits on the left from the number we do want
    uint64_t cropValue = 1;
    cropValue = cropValue << numBits;
    cropValue -= 1;
    retValue = retValue & cropValue;

    //Update the bits remaining
    mBitsRemaining -= numBits;

    return retValue;
}

int32_t BitStreamReader::getSExpGolomb()
{
    int32_t codeNum = getUExpGolomb();

    codeNum += 1;

    if (codeNum & 1) {
        return -(codeNum>>1);
    } else
    {
        return codeNum>>1;
    }
}

uint32_t BitStreamReader::getUExpGolomb()
{
    int leadingZeroBits = -1;
    for (bool b=0; !b; leadingZeroBits++) {
        b = getBits(1);
    }
    uint32_t codeNum = 1;
    codeNum = ((codeNum << leadingZeroBits) - 1);
    codeNum += getBits(leadingZeroBits);

    return codeNum;
}


size_t BitStreamReader::totalBitsRemaining()
{
    return ((mStreamLength - mCurrLoc) * 8) + mBitsRemaining;
}

