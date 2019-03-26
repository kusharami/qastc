/*----------------------------------------------------------------------------*/
/**
 *    This confidential and proprietary software may be used only as
 *    authorised by a licensing agreement from ARM Limited
 *    (C) COPYRIGHT 2011-2012 ARM Limited
 *    ALL RIGHTS RESERVED
 *
 *    The entire notice above must be reproduced on all authorised
 *    copies and copies may only be made to the extent permitted
 *    by a licensing agreement from ARM Limited.
 *
 *    @brief    Internal function and data declarations for ASTC codec.
 */
/*----------------------------------------------------------------------------*/

#ifndef ASTC_CODEC_INTERNALS_INCLUDED

#define ASTC_CODEC_INTERNALS_INCLUDED

#include "mathlib.h"
#include <cstdint>
#include <cstdlib>

#ifndef MIN
#define MIN(x, y) ((x) < (y) ? (x) : (y))
#endif

#ifndef MAX
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#endif

// Macro to silence warnings on ignored parameters.
// The presence of this macro should be a signal to look at refactoring.
#define IGNOREPARAM(param) ((void) &param)

#define astc_isnan(p) ((p) != (p))

// ASTC parameters
#define MAX_TEXELS_PER_BLOCK 216
#define MAX_WEIGHTS_PER_BLOCK 64
#define MIN_WEIGHT_BITS_PER_BLOCK 24
#define MAX_WEIGHT_BITS_PER_BLOCK 96
#define PARTITION_BITS 10
#define PARTITION_COUNT (1 << PARTITION_BITS)

// the sum of weights for one texel.
#define TEXEL_WEIGHT_SUM 16
#define MAX_DECIMATION_MODES 87
#define MAX_WEIGHT_MODES 2048

struct processed_line2
{
	float2 amod;
	float2 bs;
	float2 bis;
};
struct processed_line3
{
	float3 amod;
	float3 bs;
	float3 bis;
};
struct processed_line4
{
	float4 amod;
	float4 bs;
	float4 bis;
};

enum astc_decode_mode
{
	ASTC_DECODE_LDR_SRGB,
	ASTC_DECODE_LDR,
	ASTC_DECODE_HDR
};

/* 
    Partition table representation:
    For each block size, we have 3 tables, each with 1024 partitionings;
    these three tables correspond to 2, 3 and 4 partitions repsectively.
    For each partitioning, we have: 
    * a 4-entry table indicating how many texels there are in each of the 4 partitions.
      This may be from 0 to a very large value.
    * a table indicating the partition index of each of the texels in the block.
      Each index may be 0, 1, 2 or 3.
    * Each element in the table is an uint8_t indicating partition index (0, 1, 2 or 3)
*/

struct partition_info
{
	int partition_count;
	uint8_t texels_per_partition[4];
	uint8_t partition_of_texel[MAX_TEXELS_PER_BLOCK];
	uint8_t texels_of_partition[4][MAX_TEXELS_PER_BLOCK];

	uint64_t coverage_bitmaps
		[4]; // used for the purposes of k-means partition search.
};

/* 
   In ASTC, we don't necessarily provide a weight for every texel.
   As such, for each block size, there are a number of patterns where some texels
   have their weights computed as a weighted average of more than 1 weight.
   As such, the codec uses a data structure that tells us: for each texel, which
   weights it is a combination of for each weight, which texels it contributes to.
   The decimation_table is this data structure.
*/
struct decimation_table
{
	int num_texels;
	int num_weights;
	uint8_t texel_num_weights
		[MAX_TEXELS_PER_BLOCK]; // number of indices that go into the calculation for a texel
	uint8_t texel_weights_int[MAX_TEXELS_PER_BLOCK]
							 [4]; // the weight to assign to each weight
	float texel_weights_float[MAX_TEXELS_PER_BLOCK]
							 [4]; // the weight to assign to each weight
	uint8_t texel_weights[MAX_TEXELS_PER_BLOCK]
						 [4]; // the weights that go into a texel calculation
	uint8_t weight_num_texels
		[MAX_WEIGHTS_PER_BLOCK]; // the number of texels that a given weight contributes to
	uint8_t weight_texel
		[MAX_WEIGHTS_PER_BLOCK]
		[MAX_TEXELS_PER_BLOCK]; // the texels that the weight contributes to
	uint8_t weights_int
		[MAX_WEIGHTS_PER_BLOCK]
		[MAX_TEXELS_PER_BLOCK]; // the weights that the weight contributes to a texel.
	float weights_flt
		[MAX_WEIGHTS_PER_BLOCK]
		[MAX_TEXELS_PER_BLOCK]; // the weights that the weight contributes to a texel.
};

/* 
   data structure describing information that pertains to a block size and its associated block modes.
*/
struct block_mode
{
	int8_t decimation_mode;
	int8_t quantization_mode;
	int8_t is_dual_plane;
	int8_t permit_encode;
	int8_t permit_decode;
	float percentile;
};

struct block_size_descriptor
{
	int decimation_mode_count;
	int decimation_mode_samples[MAX_DECIMATION_MODES];
	int decimation_mode_maxprec_1plane[MAX_DECIMATION_MODES];
	int decimation_mode_maxprec_2planes[MAX_DECIMATION_MODES];
	float decimation_mode_percentile[MAX_DECIMATION_MODES];
	int permit_encode[MAX_DECIMATION_MODES];
	decimation_table decimation_tables[MAX_DECIMATION_MODES + 1];
	block_mode block_modes[MAX_WEIGHT_MODES];

	// for the k-means bed bitmap partitioning algorithm, we don't
	// want to consider more than 64 texels; this array specifies
	// which 64 texels (if that many) to consider.
	int texelcount_for_bitmap_partitioning;
	int texels_for_bitmap_partitioning[64];
};

// data structure representing one block of an image.
// it is expanded to float prior to processing to save some computation time
// on conversions to/from uint8_t (this also allows us to handle hdr textures easily)
struct imageblock
{
	float orig_data[MAX_TEXELS_PER_BLOCK * 4]; // original input data
	float work_data[MAX_TEXELS_PER_BLOCK *
		4]; // the data that we will compress, either linear or LNS (0..65535 in both cases)
	float deriv_data[MAX_TEXELS_PER_BLOCK *
		4]; // derivative of the conversion function used, used ot modify error weighting

	uint8_t rgb_lns[MAX_TEXELS_PER_BLOCK *
		4]; // 1 if RGB data are being trated as LNS
	uint8_t alpha_lns[MAX_TEXELS_PER_BLOCK *
		4]; // 1 if Alpha data are being trated as LNS
	uint8_t
		nan_texel[MAX_TEXELS_PER_BLOCK * 4]; // 1 if the texel is a NaN-texel.

	float red_min, red_max;
	float green_min, green_max;
	float blue_min, blue_max;
	float alpha_min, alpha_max;
	int grayscale; // 1 if R=G=B for every pixel, 0 otherwise

	int xpos, ypos, zpos;
	int xsize, ysize, zsize;
};

struct error_weighting_params
{
	float rgb_power;
	float rgb_base_weight;
	float rgb_mean_weight;
	float rgb_stdev_weight;
	float alpha_power;
	float alpha_base_weight;
	float alpha_mean_weight;
	float alpha_stdev_weight;
	float rgb_mean_and_stdev_mixing;
	int mean_stdev_radius;
	int enable_rgb_scale_with_alpha;
	int alpha_radius;
	int ra_normal_angular_scale;
	float block_artifact_suppression;
	float rgba_weights[4];

	float block_artifact_suppression_expanded[MAX_TEXELS_PER_BLOCK];

	// parameters that deal with heuristic codec speedups
	int partition_search_limit;
	float block_mode_cutoff;
	float texel_avg_error_limit;
	float partition_1_to_2_limit;
	float lowest_correlation_cutoff;
	int max_refinement_iters;
};

/* 
    Data structure representing error weighting for one block of an image. this is used as
    a multiplier for the error weight to apply to each color component when computing PSNR.

    This weighting has several uses: it's usable for RA, GA, BA, A weighting, which is useful
    for alpha-textures it's usable for HDR textures, where weighting should be approximately inverse to
    luminance it's usable for perceptual weighting, where we assign higher weight to low-variability
    regions than to high-variability regions. it's usable for suppressing off-edge block content in
    case the texture doesn't actually extend to the edge of the block.

    For the default case (everything is evenly weighted), every weight is 1. For the RA,GA,BA,A case,
    we multiply the R,G,B weights with that of the alpha.

    Putting the same weight in every component should result in the default case.
    The following relations should hold:

    texel_weight_rg[i] = (texel_weight_r[i] + texel_weight_g[i]) / 2
    texel_weight_lum[i] = (texel_weight_r[i] + texel_weight_g[i] + texel_weight_b[i]) / 3
    texel_weight[i] = (texel_weight_r[i] + texel_weight_g[i] + texel_weight_b[i] + texel_weight_a[i] / 4
 */

struct error_weight_block
{
	float4 error_weights[MAX_TEXELS_PER_BLOCK];
	float texel_weight[MAX_TEXELS_PER_BLOCK];
	float texel_weight_gba[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rba[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rga[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rgb[MAX_TEXELS_PER_BLOCK];

	float texel_weight_rg[MAX_TEXELS_PER_BLOCK];
	float texel_weight_rb[MAX_TEXELS_PER_BLOCK];
	float texel_weight_gb[MAX_TEXELS_PER_BLOCK];
	float texel_weight_ra[MAX_TEXELS_PER_BLOCK];

	float texel_weight_r[MAX_TEXELS_PER_BLOCK];
	float texel_weight_g[MAX_TEXELS_PER_BLOCK];
	float texel_weight_b[MAX_TEXELS_PER_BLOCK];
	float texel_weight_a[MAX_TEXELS_PER_BLOCK];

	int contains_zeroweight_texels;
};

struct error_weight_block_orig
{
	float4 error_weights[MAX_TEXELS_PER_BLOCK];
};

// enumeration of all the quantization methods we support under this format.
enum quantization_method
{
	QUANT_2 = 0,
	QUANT_3 = 1,
	QUANT_4 = 2,
	QUANT_5 = 3,
	QUANT_6 = 4,
	QUANT_8 = 5,
	QUANT_10 = 6,
	QUANT_12 = 7,
	QUANT_16 = 8,
	QUANT_20 = 9,
	QUANT_24 = 10,
	QUANT_32 = 11,
	QUANT_40 = 12,
	QUANT_48 = 13,
	QUANT_64 = 14,
	QUANT_80 = 15,
	QUANT_96 = 16,
	QUANT_128 = 17,
	QUANT_160 = 18,
	QUANT_192 = 19,
	QUANT_256 = 20
};

/* 
    In ASTC, we support relatively many combinations of weight precisions and weight transfer functions.
    As such, for each combination we support, we have a hardwired data structure.

    This structure provides the following information: A table, used to estimate the closest quantized
    weight for a given floating-point weight. For each quantized weight, the corresponding unquantized
    and floating-point values. For each quantized weight, a previous-value and a next-value.
*/

struct quantization_and_transfer_table
{
	quantization_method method;
	uint8_t unquantized_value[32]; // 0..64
	float unquantized_value_flt[32]; // 0..1
	uint8_t prev_quantized_value[32];
	uint8_t next_quantized_value[32];
	uint8_t closest_quantized_weight[1025];
};

extern const quantization_and_transfer_table quant_and_xfer_tables[12];

enum endpoint_formats
{
	FMT_LUMINANCE = 0,
	FMT_LUMINANCE_DELTA = 1,
	FMT_HDR_LUMINANCE_LARGE_RANGE = 2,
	FMT_HDR_LUMINANCE_SMALL_RANGE = 3,
	FMT_LUMINANCE_ALPHA = 4,
	FMT_LUMINANCE_ALPHA_DELTA = 5,
	FMT_RGB_SCALE = 6,
	FMT_HDR_RGB_SCALE = 7,
	FMT_RGB = 8,
	FMT_RGB_DELTA = 9,
	FMT_RGB_SCALE_ALPHA = 10,
	FMT_HDR_RGB = 11,
	FMT_RGBA = 12,
	FMT_RGBA_DELTA = 13,
	FMT_HDR_RGB_LDR_ALPHA = 14,
	FMT_HDR_RGBA = 15,
};

struct symbolic_compressed_block
{
	int error_block; // 1 marks error block, 0 marks non-error-block.
	int block_mode; // 0 to 2047. Negative value marks constant-color block (-1: FP16, -2:UINT16)
	int partition_count; // 1 to 4; Zero marks a constant-color block.
	int partition_index; // 0 to 1023
	int color_formats[4]; // color format for each endpoint color pair.
	int color_formats_matched; // color format for all endpoint pairs are matched.
	int color_values[4][12]; // quantized endpoint color pairs.
	int color_quantization_level;
	uint8_t plane1_weights
		[MAX_WEIGHTS_PER_BLOCK]; // quantized and decimated weights
	uint8_t plane2_weights[MAX_WEIGHTS_PER_BLOCK];
	int plane2_color_component; // color component for the secondary plane of weights
	int constant_color
		[4]; // constant-color, as FP16 or UINT16. Used for constant-color blocks only.
};

struct physical_compressed_block
{
	uint8_t data[16];
};

struct astc_codec_image
{
	uint8_t ***imagedata8;
	uint16_t ***imagedata16;
	int xsize;
	int ysize;
	int zsize;
	int padding;
};


// the entries here : 0=red, 1=green, 2=blue, 3=alpha, 4=0.0, 5=1.0
struct swizzlepattern
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
};

struct endpoints
{
	int partition_count;
	float4 endpt0[4];
	float4 endpt1[4];
};

struct endpoints_and_weights
{
	endpoints ep;
	float weights[MAX_TEXELS_PER_BLOCK];
	float weight_error_scale[MAX_TEXELS_PER_BLOCK];
};

struct encoding_choice_errors
{
	float
		rgb_scale_error; // error of using LDR RGB-scale instead of complete endpoints.
	float
		rgb_luma_error; // error of using HDR RGB-scale instead of complete endpoints.
	float luminance_error; // error of using luminance instead of RGB
	float alpha_drop_error; // error of discarding alpha
	float rgb_drop_error; // error of discarding rgb
	int can_offset_encode;
	int can_blue_contract;
};

#endif
