#ifndef _ASTC_ENCODE_KERNEL_H
#define _ASTC_ENCODE_KERNEL_H

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
*/
/*----------------------------------------------------------------------------*/
//===========================================================================
// Copyright (c) 2014-2017  Advanced Micro Devices, Inc. All rights reserved.
//===========================================================================

#include "ARM/astc_codec_internals.h"

struct Vec4uc
{
	Vec4uc() {}
	Vec4uc(unsigned char s)
		: x(s)
		, y(s)
		, z(s)
		, w(s)
	{
	}
	Vec4uc(unsigned char x, unsigned char y, unsigned char z, unsigned char w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{
	}

	unsigned char x, y, z, w;

	inline Vec4uc operator*(unsigned char s) const
	{
		return Vec4uc(x * s, y * s, z * s, w * s);
	}
	inline Vec4uc operator^(const Vec4uc &a) const
	{
		return Vec4uc(x ^ a.x, y ^ a.y, z ^ a.z, w ^ a.w);
	}
	inline Vec4uc operator&(const Vec4uc &a) const
	{
		return Vec4uc(x & a.x, y & a.y, z & a.z, w & a.w);
	}
	inline bool operator==(const Vec4uc &a) const
	{
		return (x == a.x && y == a.y && z == a.z && w == a.w);
	}
	inline Vec4uc operator+(const Vec4uc &a) const
	{
		return Vec4uc(x + a.x, y + a.y, z + a.z, w + a.w);
	}
};

struct afloat3
{
	afloat3()
	{
		x = 0;
		y = 0;
		z = 0;
	}
	afloat3(float s)
		: x(s)
		, y(s)
		, z(s)
	{
	}
	afloat3(float x, float y, float z)
		: x(x)
		, y(y)
		, z(z)
	{
	}
	float x, y, z;

	inline afloat3 operator*(float s) const
	{
		return afloat3(x * s, y * s, z * s);
	}
	inline float operator*(const afloat3 &a) const
	{
		return ((x * a.x) + (y * a.y) + (z * a.z));
	}
	inline afloat3 operator+(const afloat3 &a) const
	{
		return afloat3(x + a.x, y + a.y, z + a.z);
	}
	inline afloat3 operator-(const afloat3 &a) const
	{
		return afloat3(x - a.x, y - a.y, z - a.z);
	}
};

namespace ASTC_Encoder
{
#define ANGULAR_STEPS 88
#define SINCOS_STEPS 64

extern const float angular_steppings[ANGULAR_STEPS];
extern const int steps_of_level[];

#define ENABLE_3_PARTITION_CODE
#define ENABLE_4_PARTITION_CODE

// #define USE_RA_NORMAL_ANGULAR_SCALE
// #define USE_RGB_SCALE_WITH_ALPHA
// #define USE_PERFORMM_SRGB_TRANSFORM

#define ENABLE_64Bit_Support // See definitions of coverage_bitmaps in partition_info  based on uint64_cl

#ifdef ENABLE_64Bit_Support
#define COVERAGE_BITMAPS_MAX 64
#else
#define COVERAGE_BITMAPS_MAX 32
#endif

// #define ASTC_ENABLE_3D_SUPPORT     : Incomplete code do not enable!

#define FLOAT_n4 1e-4f
#define FLOAT_n7 1e-7f
#define FLOAT_n10 1e-10f
#define FLOAT_n11 1e-11f
#define FLOAT_n12 1e-12f
#define FLOAT_n17 1e-17f
#define FLOAT_n18 1e-18f
#define FLOAT_n20 1e-20f
#define FLOAT_n30 1e-30f

#define FLOAT_9 1e9f
#define FLOAT_10 1e10f
#define FLOAT_15 1e15f
#define FLOAT_20 1e20f
#define FLOAT_29 1e29f
#define FLOAT_30 1e30f
#define FLOAT_34 1e34f
#define FLOAT_35 1e35f
#define FLOAT_38 1e38f

#ifndef M_PI
#define M_PI 3.14159265358979323846264338
#endif

typedef signed char int8_t;
typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
#ifdef ENABLE_64Bit_Support
typedef unsigned long long
	uint64_cl; // See notes on the definition should be 64bit
#else
typedef unsigned int uint64_cl;
#endif
typedef unsigned short CGU_SHORT;
typedef unsigned int CGU_UINT;

// Pixel format AABBGGRR,  AA = Alpha 1 Byte  BB = Blue, GG = Green, RR = Red
typedef ushort4 pixel_type;

// The compressed and encoded block of pixels.
typedef struct
{
	uint32_t m_bits[4]; // 128 bits
} encoded_block;

// block of pixels
#define MAX_NUM_PIXELS_PER_BLOCK 144 // 12x12 pixels per block max
#define BYTES_PER_DESTINATION_BLOCK 16
#define BYTEPP 4 // Pixel = RGBA

#define TEXEL_WEIGHT_SUM 16
#define MAX_TEXELS_PER_BLOCK 216
#define MAX_TEXELS_PER_BLOCK4 864 // MAX_TEXELS_PER_BLOCK * 4

#define MAX_WEIGHTS_PER_BLOCK 64
#define MAX_DECIMATION_MODES 87
#define MAX_WEIGHT_MODES 2048

#define MIN_WEIGHT_BITS_PER_BLOCK 24
#define MAX_WEIGHT_BITS_PER_BLOCK 96

typedef struct
{
	float4 a;
	float4 b;
} line4;

typedef struct
{
	float3 a;
	float3 b;
} line3;

typedef struct
{
	float2 a;
	float2 b;
} line2;

typedef struct
{
	float3 amod;
	float3 bs;
	float3 bis;
} processed_line3;

typedef struct
{
	float2 amod;
	float2 bs;
	float2 bis;
} processed_line2;

typedef struct
{
	float4 amod;
	float4 bs;
	float4 bis;
} processed_line4;

extern const float dummy_percentile_table_3d[2048];

typedef union if32_
{
	uint32_t u;
	int32_t s;
	float f;
} if32;

/*
In ASTC, we support relatively many combinations of weight precisions and weight transfer functions.
As such, for each combination we support, we have a hardwired data structure.

This structure provides the following information: A table, used to estimate the closest quantized
weight for a given floating-point weight. For each quantized weight, the corresponding unquantized
and floating-point values. For each quantized weight, a previous-value and a next-value.
*/

//=======================================================================================================================

// quantization_mode_table[integercount/2][bits] gives
// us the quantization level for a given integer count and number of bits that
// the integer may fit into. This is needed for color decoding,
// and for the color encoding.
typedef int quantization_mode_table_t[128];
typedef float sincos_table_t[ANGULAR_STEPS];

extern const quantization_mode_table_t *quantization_mode_table; //[17]
extern const sincos_table_t *sin_table;
extern const sincos_table_t *cos_table;
extern const float *stepsizes; //[ANGULAR_STEPS];
extern const float *stepsizes_sqr; //[ANGULAR_STEPS];
extern const int *max_angular_steps_needed_for_quant_level; //[13]

typedef struct
{
	unsigned int m_src_width; // Original source width
	unsigned int m_src_height; // Original source height
	unsigned int m_xdim; // Compression block size width
	unsigned int m_ydim; // Compression block size height
	unsigned int m_zdim; // Compression block size depth

	int batch_size;

	int m_rgb_force_use_of_hdr;
	int m_alpha_force_use_of_hdr;
	int m_perform_srgb_transform;
	int m_texels_per_block; //
	unsigned int m_width_in_blocks; //
	unsigned int m_height_in_blocks; //

	// User settings
	astc_decode_mode m_decode_mode;
	error_weighting_params m_ewp;
	const block_size_descriptor *bsd;
	float m_Quality;
	const partition_info *const *partition_tables;
} ASTC_Encode;

struct compress_symbolic_block_buffers
{
	float decimated_weights[2 * MAX_DECIMATION_MODES * MAX_WEIGHTS_PER_BLOCK];
	uint8_t u8_quantized_decimated_quantized_weights[2 * MAX_WEIGHT_MODES *
		MAX_WEIGHTS_PER_BLOCK];
	float decimated_quantized_weights[2 * MAX_DECIMATION_MODES *
		MAX_WEIGHTS_PER_BLOCK];
	float flt_quantized_decimated_quantized_weights[2 * MAX_WEIGHT_MODES *
		MAX_WEIGHTS_PER_BLOCK];	
};

extern void imageblock_initialize_work_from_orig(
	imageblock *pb, int pixelcount);
extern int compute_ise_bitcount(int items, quantization_method quant);

void update_imageblock_flags(imageblock *pb, ASTC_Encode *ASTC_Encode);

extern void decompress_symbolic_block(
	symbolic_compressed_block *scb, imageblock *blk, ASTC_Encode *ASTCEncode);

extern float compress_symbolic_block(
	imageblock *blk, symbolic_compressed_block *scb, ASTC_Encode *ASTCEncode,
		compress_symbolic_block_buffers *buffers);

extern physical_compressed_block symbolic_to_physical(
	symbolic_compressed_block *sc, ASTC_Encode *ASTCEncode);

} // ASTC_ENCODE namespace

#endif
