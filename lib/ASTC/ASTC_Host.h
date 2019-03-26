//=====================================================================
// Copyright (c) 2016    Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
/// \file ASTC_HOST.h
//
//=====================================================================

#ifndef _ASTC_HOST_H
#define _ASTC_HOST_H

#include "ASTC_Encode_Kernel.h"
#include "ARM/astc_codec_internals.h"

#define __constant const
#define __kernel
#define __global

#include <cstdint>

namespace ASTC_Encoder
{
bool init_ASTC(__global ASTC_Encode *ASTCEncode);

extern uint16_t unorm16_to_sf16(uint16_t p);
extern uint16_t lns_to_sf16(uint16_t p);
extern void find_number_of_bits_trits_quints(
	int quantization_level, int *bits, int *trits, int *quints);
extern void luminance_unpack(
	int input[2], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void luminance_delta_unpack(
	int input[2], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_luminance_small_range_unpack(
	int input[2], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_luminance_large_range_unpack(
	int input[2], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void luminance_alpha_unpack(
	int input[4], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void luminance_alpha_delta_unpack(
	int input[4], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void rgb_scale_unpack(
	int input[4], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void rgb_scale_alpha_unpack(
	int input[6], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_rgbo_unpack3(
	int input[4], int quantization_level, ushort4 *output0, ushort4 *output1);
extern int rgb_unpack(
	int input[6], int quantization_level, ushort4 *output0, ushort4 *output1);
extern int rgb_delta_unpack(
	int input[6], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_rgb_unpack3(
	int input[6], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void rgba_unpack(
	int input[8], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void rgba_delta_unpack(
	int input[8], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_rgb_ldr_alpha_unpack3(
	int input[8], int quantization_level, ushort4 *output0, ushort4 *output1);
extern void hdr_rgb_hdr_alpha_unpack3(
	int input[8], int quantization_level, ushort4 *output0, ushort4 *output1);

} // namespace CMP_Kernel

//----------------------------------------------
// CPU code

using astc_codec_image_cpu = astc_codec_image;

typedef uint16_t sf16;

// on conversions to/from uint8_t (this also allows us to handle hdr textures easily)
using imageblock_cpu = imageblock;

using symbolic_compressed_block_cpu = symbolic_compressed_block;

// the entries here : 0=red, 1=green, 2=blue, 3=alpha, 4=0.0, 5=1.0
using swizzlepattern_cpu = swizzlepattern;

using decimation_table_cpu = decimation_table;

/*
    data structure describing information that pertains to a block size and its associated block modes.
    */
using block_mode_cpu = block_mode;

using block_size_descriptor_cpu = block_size_descriptor;

using physical_compressed_block_cpu = physical_compressed_block;

using partition_info_cpu = partition_info;

astc_codec_image_cpu *allocate_image_cpu(
	int bitness, int xsize, int ysize, int zsize, int padding);

void initialize_image_cpu(astc_codec_image_cpu *img);

void physical_to_symbolic_cpu(int xdim, int ydim, int zdim,
	physical_compressed_block_cpu pb, symbolic_compressed_block_cpu *res);


void write_imageblock_cpu(astc_codec_image_cpu *img, const imageblock_cpu *pb,
	int xdim, int ydim, int zdim, int xpos, int ypos, int zpos,
	swizzlepattern_cpu swz);

void destroy_image_cpu(astc_codec_image_cpu *img);


void fetch_imageblock_cpu(const astc_codec_image_cpu *img, imageblock_cpu *pb,
	// position in texture.
	int xpos, int ypos, int zpos,
	__global ASTC_Encoder::ASTC_Encode *ASTCEncode);

#endif
