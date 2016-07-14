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


#ifndef __AppStreamSampleClient__NALUtils__
#define __AppStreamSampleClient__NALUtils__

#import "BitStreamReader.h"

typedef enum {
    NAL_TYPE_UNDEFINED              = 0,
    NAL_TYPE_NON_IDR_SLICE          = 1, //VCL-type
    NAL_TYPE_SLICE_DATA_A           = 2, //VCL-type
    NAL_TYPE_SLICE_DATA_B           = 3, //VCL-type
    NAL_TYPE_SLICE_DATA_C           = 4, //VCL-type
    NAL_TYPE_IDR_SLICE              = 5, //VCL-type
    NAL_TYPE_SEI                    = 6,
    NAL_TYPE_SPS                    = 7,
    NAL_TYPE_PPS                    = 8,
    NAL_TYPE_AUD                    = 9,
    NAL_TYPE_END_OF_SEQUENCE        = 10,
    NAL_TYPE_END_OF_STREAM          = 11,
    NAL_TYPE_FILLER_DATA            = 12,
    NAL_TYPE_SPS_EXTENSION          = 13,
    NAL_TYPE_PREFIX_NAL             = 14,
    NAL_TYPE_SUBSET_SPS             = 15,
    //16-18 are reserved
    NAL_TYPE_CODED_AUX_NO_PARTITION = 19,
    NAL_TYPE_CODED_SLICE_EXTENSION  = 20,
    NAL_TYPE_CODED_SLICE_EXT_DEPTH  = 21
    //22-23 are reserved
    //24-31 are unspecified
} nalType;


typedef struct
{
    uint8_t profile;
    uint8_t level;
    uint8_t seq_parameter_set_id;
    uint8_t chroma_format;
    bool separate_color_plane;
    uint8_t bit_depth_luma_minus8;
    uint8_t bit_depth_chroma_minus8;
    bool qpprime_y_zero_transform_bypass_flag;
    bool seq_scaling_matrix_present_flag;
    uint8_t log2_max_frame_num_minus4;
    uint8_t pic_order_cnt_type;
    uint8_t log2_max_pic_order_cnt_lsb_minus4;
    bool delta_pic_order_always_zero_flag;
    int8_t offset_for_non_ref_pic;
    int8_t offset_for_top_to_bottom_field;
    uint8_t num_ref_frames_in_pic_order_cnt_cycle;
    uint8_t max_num_ref_frames;
    bool gaps_in_frame_num_value_allowed_flag;
    uint8_t pic_width_in_mbs_minus1;
    uint8_t pic_height_in_map_units_minus1;
    bool frame_mbs_only_flag;
    bool mb_adaptive_frame_field_flag;
    bool direct_8x8_inference_flag;
    bool frame_cropping_flag;
    uint8_t frame_crop_left_offset;
    uint8_t frame_crop_right_offset;
    uint8_t frame_crop_top_offset;
    uint8_t frame_crop_bottom_offset;
    bool vui_parameters_present_flag;
} sps;

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
BOOL getNALUnit( nalType whichNAL, uint8_t *dataStart, uint8_t *dataEnd, uint8_t *&nalUnitStart, uint32_t &nalUnitLength);

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
uint8_t * findNALUnitStart( uint8_t *start, uint8_t *end, uint8_t &nalUnitHeaderLength);

/**
 * Extracts the RBSP from an input NAL unit. RBSP has the data emulation
 * prevention bits stripped out.
 *
 * @param[in] dataStart pointer to the start of the NAL unit's data
 * @param[in] dataLength length of the NAL unit
 * @param[out] rbspOut on return will point to the resulting rbsp data. Will
 * need to be free'd by the caller
 * @param[out] rbspLength on return will hold the length of the rbsp
 */
void extractRBSP(uint8_t *dataStart, uint32_t dataLength, uint8_t *&rbspOut, uint32_t &rbspLength);

/**
 * Creates an avcC style extraData based on the sps and pps passed in
 *
 * @param[out] extraData pointer to the resulting extraData
 * @param[in] spsData pointer to the SPS NAL
 * @param[in] spsLength length of the SPS NAL passed in
 * @param[in] ppsData pointer to the PPS NAL
 * @param[in] ppsLength length of the PPS NAL passed in
 */
void makeExtraData( uint8_t *&extraData, uint8_t *spsData, uint8_t spsLength, uint8_t *ppsData, uint8_t ppsLength);


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
                                     uint8_t *&newFrameData, uint32_t &newFrameLength);

/**
 * Decodes the scaling_list of the SPS when parsing
 * Should not need to be called other than from the parseSPS function
 * Does not return anything because we don't actually care about the scaling_list
 * we just need to work through the bits in the SPS NAL Unit
 *
 * @param[in] bitReader pointer to the BitStreamReader being used
 * @param[in] sizeOfScalingList size of the scaling list needed to parse
 */
void scalingList(BitStreamReader *bitReader, uint8_t sizeOfScalingList);

/**
 * Parses the SPS NAL Unit into a sps struct
 *
 * @param[in] spsData pointer to the SPS NAL unit data
 * @param[in] spsLength length of the SPS NAL unit data
 * @return sps struct holding the parsed SPS NAL unit
 */
sps parseSPS(uint8_t *spsData, uint32_t spsLength);

/**
 * Calculates the video resolution from the SPS struct given
 *
 * @param[in] theSPS filled in sps struct
 * @param[out] videoWidth on return will contain the horizontal size of the video
 * @param[out] videoHeight on return will contain the vertical size of the video
 */
void getVideoSizeFromSPS(sps theSPS, uint32_t &videoWidth, uint32_t &videoHeight);


#endif /* defined(__AppStreamSampleClient__NALUtils__) */
