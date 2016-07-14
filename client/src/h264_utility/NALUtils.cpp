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


#include "NALUtils.h"

#undef LOG_TAG
#define LOG_TAG "NALUtils"
#include "log.h"


/**
 * Finds and returns the NAL unit with the given type.
 *
 * @param[in] whichNAL The type of NAL unit being searched for
 * @param[in] dataStart pointer to the beginning of the video frame data
 * @param[in] dataEnd pointer to the end of the video frame data
 * @param[out] nalUnitStart on success will point to the beginning of the NAL unit
 * @param[out] nalUnitLength on success will have the length of the NAL unit in bytes
 * @return TRUE if the NAL unit was found FALSE if it was not
 */
BOOL getNALUnit( nalType whichNAL, uint8_t *dataStart, uint8_t *dataEnd,
                uint8_t *&nalUnitStart, uint32_t &nalUnitLength)
{
    uint8_t *start = dataStart;
    uint8_t nalHeaderLength = 0;
    
    while (start < dataEnd && start != NULL) {
        start = findNALUnitStart(start, dataEnd, nalHeaderLength);
        
        if (start == NULL) {
            //Didn't find a NAL unit
            break;
        }
        //Found a NAL but is it the right type?
        
        //Mask out the first byte to determine which NAL type it is
        uint8_t startType = start[0] & 0x1f;
        
        if (startType == whichNAL) {
            //It is the right type
            nalUnitStart = start;
            
            //Get the end by finding the start of the next NAL
            start = findNALUnitStart(start, dataEnd, nalHeaderLength);
            if (start == NULL) {
                //This was the last NAL unit so it ends at dataEnd
                nalUnitLength = uint32_t(dataEnd - nalUnitStart);
            } else
            {
                //It ends at the start of the header for the next one
                uint8_t *startLessHeader = (start - nalHeaderLength);
                nalUnitLength = uint32_t(startLessHeader - nalUnitStart);
            }
            
            return true;
        }
    }
    
    //Didn't find a NAL (or at least one of that type)
    return false;
}

/**
 * NALUnits start with 2 or 3 0x00 bytes followed by 0x01
 * This function returns the location of the start of the next
 * NAL unit. The first byte pointed to is the type byte
 * and can be used to determine which type of NAL it is
 *
 * @param[in] start pointer to the beginning of the video frame data
 * @param[in] end pointer to the end of the video frame data
 * @param[out] nalUnitHeaderLength length of the NAL header (should be be 3 or 4 if a NAL is found)
 * @return pointer to location of the first byte in the NAL or NULL if no NAL is found
 */
uint8_t * findNALUnitStart( uint8_t *start, uint8_t *end, uint8_t &nalUnitHeaderLength)
{
    while (end > start)
    {
        //Find the first 0 in the data
        start = (uint8_t*)memchr(start,0,end-start);
        
        // no more zeros? or no more room for a header + data? None to find.
        if (start==NULL || (end-start)<=5 )
        {
            return NULL;
        }
        else
        {
            if (start[1] != 0)
            {
                //It's not a 0 0 so push out our start loc and try again
                start++;
                continue;
            }
            
            // 0 0 1 is valid
            if (start[2] == 1)
            {
                nalUnitHeaderLength = 3;
                return start + 3;
            }
            
            if (start[2] != 0)
            {
                //Not a 0 0 0 so push out start loc and try again
                start ++;
                continue;
            }
            
            // 0 0 0 1 is valid
            if (start[3] == 1)
            {
                nalUnitHeaderLength = 4;
                return start + 4;
            }
            
            //Lots of 0's but we need 001 so keep looking
            start ++;
        }
        
        return start;
    }
    
    return NULL;
}

/**
 * Extracts the RBSP from an input NAL unit. RBSP has the data emulation
 * prevention bits stripped out.
 *
 * @param[in] dataStart pointer to the start of the NAL unit's data
 * @param[in] dataLength length of the NAL unit
 * @param[out] rbspOut on return will point to the resulting rbsp data
 * @param[out] rbspLength on return will hold the length of the rbsp
 */
void extractRBSP(uint8_t *dataStart, uint32_t dataLength, uint8_t *&rbspOut, uint32_t &rbspLength)
{
    //Start off with a 0 length
    rbspLength = 0;
    
    //Create a temp buffer to put the rbsp data into
    uint8_t rbspOutData[dataLength];
    memset(&rbspOutData, 0x00, dataLength);
    
    int currLocInData = 0;
    
    //First byte is the NAL type so put it in the rbsp
    rbspOutData[rbspLength++] = dataStart[currLocInData++];
    
    //Look through the rest of the SPS and remove any emulation prevention bytes
    // (0x00 0x00 0x03)
    while (currLocInData < dataLength)
    {
        if ((currLocInData + 2) < dataLength)
        {
            if (dataStart[currLocInData] == 0x0 && dataStart[currLocInData+1] == 0x0 && dataStart[currLocInData+2] == 0x03)
            {
                //It is an emulation prevention bit
                rbspOutData[rbspLength++] = dataStart[currLocInData++];
                rbspOutData[rbspLength++] = dataStart[currLocInData++];
                //Skip the next byte since it's the 0x03
                currLocInData++;
            } else
            {
                //Not EPB so copy through the byte
                rbspOutData[rbspLength++] = dataStart[currLocInData++];
            }
        } else
        {
            //Almost at the end copy through the byte
            rbspOutData[rbspLength++] = dataStart[currLocInData++];
        }
    }
    
    //Put the rbsp in the out variable and return it
    rbspOut = (uint8_t*) malloc(rbspLength);
    memcpy(rbspOut, rbspOutData, rbspLength);
}


/**
 * Creates an avcC style extraData based on the sps and pps passed in
 *
 * @param[out] extraData pointer to the resulting extraData
 * @param[in] spsData pointer to the SPS NAL
 * @param[in] spsLength length of the SPS NAL passed in
 * @param[in] ppsData pointer to the PPS NAL
 * @param[in] ppsLength length of the PPS NAL passed in
 */
void makeExtraData( uint8_t *&extraData, uint8_t *spsData, uint8_t spsLength,
                   uint8_t *ppsData, uint8_t ppsLength)
{
    uint8_t currIndex = 0;
    extraData[currIndex++] = 1; //avcC type 1
    extraData[currIndex++] = spsData[1]; //profile
    extraData[currIndex++] = spsData[2]; //compatibility
    extraData[currIndex++] = spsData[3]; //level
    extraData[currIndex++] = 0xFC | 3; // reserved 6-bits, NALU length size - 1 (2 bits)
    extraData[currIndex++] = 0xE0 | 1; // reserved 3-bits, number of SPS (5 bits)
    
    uint16_t spsLength16;
    spsLength16 = htons(spsLength);
    memcpy(&extraData[currIndex], &spsLength16, 2); //SPS length in 2 bytes
    currIndex += 2;
    
    memcpy(&extraData[currIndex], spsData, spsLength); //Copy the SPS
    currIndex += spsLength;
    
    extraData[currIndex++] = 0x01; // number of PPS frames (1)
    uint16_t ppsLength16 = htons(ppsLength);
    memcpy(&extraData[currIndex], &ppsLength16, 2); //PPS length in 2 bytes
    currIndex += 2;
    
    memcpy(&extraData[currIndex], ppsData, ppsLength); //Copy the PPS
    
    return;
}


/**
 * Converts the passed in H.264 frame data into one that encodes each NAL size
 * as a 4-byte size instead of a NAL header (00 00 01).
 *
 * @param[in] dataStart pointer to the beginning of the frame data
 * @param[in] dataSize size of the data pointed at by dataStart
 * @param[out] newFrameData pointer to the start of the restructured H.264 frame data
 * @param[out] newFrameLength length of the new frame
 * @return TRUE if a frame was created, FALSE otherwise
 */
BOOL convertToSizeEncodedVideoFrame ( uint8_t *dataStart, uint32_t dataSize,
                                     uint8_t *&newFrameData, uint32_t &newFrameLength)
{
    //Make sure we know where the incoming data ends
    uint8_t *dataEnd = dataStart + dataSize;
    
    //Keep track of the buffer size so it can be resized larger if needed
    static uint32_t currBufferSize = 0;
    static uint8_t *frameBuffer = NULL;
    
    //Make sure the frameBuffer is large enough to store the data with a bit of
    // extra padding since we will need to add space for the NAL headers we inject
    if (currBufferSize < (dataSize + 100)) {
        //It's too small so free our old buffer and create a new one
        if (frameBuffer != NULL) {
            free(frameBuffer);
        }
        
        currBufferSize = dataSize + 100;
        frameBuffer = (uint8_t*)malloc(currBufferSize);
        NSLog(@"IncreasingBufferTo: %i", currBufferSize);
    }
    //0 out the buffer
    memset(frameBuffer, 0, currBufferSize);
    
    
    uint32_t currBufferLen = 0;
    //Go through the data looking for NALs
    while (dataStart < dataEnd) {
        uint8_t headerLength;
        uint8_t *nalStart;
        
        nalStart = findNALUnitStart(dataStart, dataEnd, headerLength);
        
        if (nalStart == NULL) {
            //No NAL unit
            break;
        }
        
        //Update dataStart location
        dataStart = nalStart;
        
        //We found a NAL unit so check the type
        uint8_t nalType = nalStart[0] & 0x1F;
        
        if (nalType == NAL_TYPE_SPS) {
            //SPS
            LOGV("Found SPS");
            continue;
        } else if (nalType == NAL_TYPE_PPS) {
            //PPS
            LOGV("Found PPS");
            continue;
        }
        if (nalType < NAL_TYPE_NON_IDR_SLICE || nalType > NAL_TYPE_IDR_SLICE) {
            //Not a VCL NAL type so skip it and go to the next one
            LOGV("Not a VCL NAL: %i", nalType);
            continue;
        }
        
        //We have a VCL NAL so we want this in our frame data
        
        //Find the end of this one
        uint8_t nextNALHeaderLen;
        uint32_t nalUnitLength;
        uint8_t *nextNALStart = findNALUnitStart(nalStart, dataEnd, nextNALHeaderLen);
        if (nextNALStart == NULL) {
            //This was the last NAL unit so it ends at dataEnd
            nalUnitLength = (uint32_t)(dataEnd - nalStart);
        } else
        {
            uint8_t *startLessHeader = (nextNALStart - nextNALHeaderLen);
            uint32_t theLength = (uint32_t)(startLessHeader - nalStart);
            nalUnitLength = theLength;
            
            //Save some searching time for the next loop by updating dataStart
            dataStart = startLessHeader;
        }
        
        //We padded our buffer with 100 extra bytes. As long as there aren't
        // 100 NALs in the frame we shouldn't run out of space, but just in
        // case let's double check
        if ((currBufferLen + nalUnitLength + 4) >= currBufferSize) {
            //Something must be wrong becaues we don't have space for this
            // More than 100 NALs - crazy!
            //However we may still have some valid video data so we will return
            // the NALs we did copy in
            LOGE("Overran the buffer when converting to size encoded frame");
            break;
        }
        
        //Put the length of the NAL into the buffer (Big Endian)
        uint32_t nalUnitLengthBigEndian = CFSwapInt32HostToBig(nalUnitLength);
        memcpy(&frameBuffer[currBufferLen], &nalUnitLengthBigEndian, 4);
        currBufferLen += 4;
        
        //Put the NAL data itself into the buffer
        memcpy(&frameBuffer[currBufferLen], nalStart, nalUnitLength);
        currBufferLen += nalUnitLength;
    }
    
    
    if (currBufferLen <= 0) {
        //We didn't find any frame data
        return false;
    } else
    {
        newFrameData = frameBuffer;
        newFrameLength = currBufferLen;
        return true;
    }
}


/**
 * Decodes the scaling_list of the SPS when parsing
 * Should not need to be called other than from the parseSPS function
 * Does not return anything because we don't actually care about the scaling_list
 * we just need to work through the bits in the SPS NAL Unit
 *
 * @param[in] bitReader pointer to the BitStreamReader being used
 * @param[in] sizeOfScalingList size of the scaling list needed to parse
 */
void scalingList(BitStreamReader *bitReader, uint8_t sizeOfScalingList)
{
    int lastScale = 8;
    int nextScale = 8;
    for (int j=0; j<sizeOfScalingList; j++) {
        if (nextScale != 0) {
            int8_t delta_scale = bitReader->getSExpGolomb();
            nextScale = (lastScale + delta_scale + 256) % 256;
        }
        lastScale = nextScale;
    }
}

/**
 * Parses the SPS NAL Unit into a sps struct
 *
 * @param[in] spsData pointer to the SPS NAL unit data
 * @param[in] spsLength length of the SPS NAL unit data
 * @return sps struct holding the parsed SPS NAL unit
 */
sps parseSPS(uint8_t *spsData, uint32_t spsLength)
{
    uint8 *spsRBSPData = NULL;
    uint32_t spsRBSPLength = 0;
    extractRBSP(spsData, spsLength, spsRBSPData, spsRBSPLength);
    
    BitStreamReader *bitReader = new BitStreamReader(spsRBSPData, spsRBSPLength);
    
    //We now have the rbsp
    sps theSPS;
    
    //Read the first byte (which is just the SPS type)
    bitReader->getBits(8);
    //Second byte is the profile
    theSPS.profile = bitReader->getBits(8);
    //Third byte is constraints and reserved zero bits
    bitReader->getBits(8);
    //Fourth byte is the level
    theSPS.level = bitReader->getBits(8);
    theSPS.seq_parameter_set_id = bitReader->getUExpGolomb();
    
    if (theSPS.profile == 100 || theSPS.profile == 110 || theSPS.profile == 122 || theSPS.profile == 244 || theSPS.profile == 244 || theSPS.profile == 44 || theSPS.profile == 83 || theSPS.profile == 86 || theSPS.profile == 118 || theSPS.profile == 128 || theSPS.profile == 138) {
        theSPS.chroma_format = bitReader->getUExpGolomb();
        if (theSPS.chroma_format == 3) {
            theSPS.separate_color_plane = bitReader->getBits(1);
        }
        theSPS.bit_depth_luma_minus8 = bitReader->getUExpGolomb();
        theSPS.bit_depth_chroma_minus8 = bitReader->getUExpGolomb();
        theSPS.qpprime_y_zero_transform_bypass_flag = bitReader->getBits(1);
        theSPS.seq_scaling_matrix_present_flag = bitReader->getBits(1);
        if (theSPS.seq_scaling_matrix_present_flag) {
            for (int i=0;i<((theSPS.chroma_format != 3)?8:12);i++) {
                bool seq_scaling_flag = bitReader->getBits(1);
                if (seq_scaling_flag) {
                    if (i < 6) {
                        scalingList(bitReader, 16);
                    } else
                    {
                        scalingList(bitReader, 64);
                    }
                }
            }
        }
    }
    theSPS.log2_max_frame_num_minus4 = bitReader->getUExpGolomb();
    theSPS.pic_order_cnt_type = bitReader->getUExpGolomb();
    if (theSPS.pic_order_cnt_type == 0) {
        theSPS.log2_max_pic_order_cnt_lsb_minus4 = bitReader->getUExpGolomb();
    } else if (theSPS.pic_order_cnt_type == 1)
    {
        theSPS.delta_pic_order_always_zero_flag = bitReader->getBits(1);
        theSPS.offset_for_non_ref_pic = bitReader->getSExpGolomb();
        theSPS.offset_for_top_to_bottom_field = bitReader->getSExpGolomb();
        theSPS.num_ref_frames_in_pic_order_cnt_cycle = bitReader->getUExpGolomb();
        for (int i=0; i < theSPS.num_ref_frames_in_pic_order_cnt_cycle; ++i) {
            //offset_for_ref_frame[i] = se(v)
            bitReader->getSExpGolomb();
        }
    }
    
    theSPS.max_num_ref_frames = bitReader->getUExpGolomb();
    theSPS.gaps_in_frame_num_value_allowed_flag = bitReader->getBits(1);
    theSPS.pic_width_in_mbs_minus1 = bitReader->getUExpGolomb();
    theSPS.pic_height_in_map_units_minus1 = bitReader->getUExpGolomb();
    theSPS.frame_mbs_only_flag = bitReader->getBits(1);
    if (!theSPS.frame_mbs_only_flag) {
        theSPS.mb_adaptive_frame_field_flag = bitReader->getBits(1);
    }
    theSPS.direct_8x8_inference_flag = bitReader->getBits(1);
    theSPS.frame_cropping_flag = bitReader->getBits(1);
    if (theSPS.frame_cropping_flag) {
        theSPS.frame_crop_left_offset = bitReader->getUExpGolomb();
        theSPS.frame_crop_right_offset = bitReader->getUExpGolomb();
        theSPS.frame_crop_top_offset = bitReader->getUExpGolomb();
        theSPS.frame_crop_bottom_offset = bitReader->getUExpGolomb();
    }
    theSPS.vui_parameters_present_flag = bitReader->getBits(1);
    
    if (theSPS.vui_parameters_present_flag)
    {
        LOGE("VUI Parameters parsing not implemented");
    }
    
    
    //Clean up the SPS RBSP data
    if (spsRBSPData != NULL) {
        free(spsRBSPData);
    }
    
    return theSPS;
}

/**
 * Calculates the video resolution from the SPS struct given
 *
 * @param[in] theSPS filled in sps struct
 * @param[out] videoWidth on return will contain the horizontal size of the video
 * @param[out] videoHeight on return will contain the vertical size of the video
 */
void getVideoSizeFromSPS(sps theSPS, uint32_t &videoWidth, uint32_t &videoHeight)
{
    videoWidth = ((theSPS.pic_width_in_mbs_minus1 + 1) * 16);
    if (theSPS.frame_cropping_flag)
    {
        videoWidth -= (theSPS.frame_crop_left_offset * 2 + theSPS.frame_crop_right_offset * 2);
    }
    
    videoHeight = ((2 - theSPS.frame_mbs_only_flag) * (theSPS.pic_height_in_map_units_minus1 + 1) * 16);
    if (theSPS.frame_cropping_flag) {
        videoHeight -= (theSPS.frame_crop_top_offset * 2 + theSPS.frame_crop_bottom_offset * 2);
    }
}