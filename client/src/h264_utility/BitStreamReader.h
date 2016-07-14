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


#ifndef __VDADecoderDetector__BitStreamReader__
#define __VDADecoderDetector__BitStreamReader__

#include <stdint.h>

class BitStreamReader {
    /**
     * Pointer to the bit stream
     */
    uint8_t     *mStreamData;
    
    /**
     * Length of the bit stream
     */
    size_t      mStreamLength;
    
    /**
     * Current location in the bit stream (# of bytes in)
     */
    uint32_t    mCurrLoc;
    
    /**
     * Bits remaining in the current byte
     */
    uint8_t     mBitsRemaining;
    
    /**
     * Current Byte being examined
     */
    uint64_t     mCurrBits;
    
public:
    /**
     * Constructor.
     *
     * @param[in] streamData pointer to the bit stream to parse
     * @param[in] streamLength length in bytes of the bit stream
     */
    BitStreamReader(uint8_t *streamData, size_t streamLength);
    
    /**
     * An empty virtual destructor. Ensures that derived classes are destroyed correctly.
     */
    virtual ~BitStreamReader() { };
    
    /**
     * Read the given number of bits (up to 32)
     *
     * @return int holding the desired bits
     */
    uint32_t getBits(uint8_t numBits);
    
    /**
     * Get an unsigned exp golomb encoded value
     *
     * @return the unsigned exp golomb encoded value
     */
    uint32_t getUExpGolomb();
    
    /**
     * Get a signed exp golomb encoded value
     *
     * @return the signed exp golomb encoded value
     */
    int32_t getSExpGolomb();
    
    /**
     * Get the total bits remaining to the end of the stream
     *
     * @return total bits remaining to the end of the stream
     */
    size_t totalBitsRemaining();
};

#endif /* defined(__VDADecoderDetector__BitStreamReader__) */
