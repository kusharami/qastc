//===============================================================================
// Copyright (c) 2007-2016  Advanced Micro Devices, Inc. All rights reserved.
// Copyright (c) 2004-2006 ATI Technologies Inc.
//===============================================================================
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
// ASTC_Decode.cpp : A reference decoder for ASTC
//

#include "ASTC/ASTC_Decode.h"

#include <cassert>
#include <cstring>

ASTCBlockDecoder::ASTCBlockDecoder(
	ASTC_Encoder::ASTC_Encode *codec, BYTE BlockWidth, BYTE BlockHeight)
	: codec(codec)
	, img(allocate_image_cpu(8, BlockWidth, BlockHeight, 1, 0))
{
}

ASTCBlockDecoder::~ASTCBlockDecoder()
{
	destroy_image_cpu(img);
}

void ASTCBlockDecoder::decompress(CMP_COLOR out[], const BYTE in[])
{
	DecompressBlock(img, out, in, codec);
}

void ASTCBlockDecoder::DecompressBlock(BYTE BlockWidth, BYTE BlockHeight,
	CMP_COLOR out[], const BYTE in[], ASTC_Encoder::ASTC_Encode *encoder)
{
	// Results Buffer
	astc_codec_image_cpu *img =
		allocate_image_cpu(8, BlockWidth, BlockHeight, 1, 0);

	DecompressBlock(img, out, in, encoder);

	destroy_image_cpu(img);
}

void ASTCBlockDecoder::DecompressBlock(astc_codec_image_cpu *img,
	CMP_COLOR out[], const BYTE in[], ASTC_Encoder::ASTC_Encode *encoder)
{
	assert(img->imagedata8);
	assert(img->zsize == 1);
	assert(img->padding == 0);
	int BlockWidth = img->xsize;
	int BlockHeight = img->ysize;

	initialize_image_cpu(img);

	physical_compressed_block_cpu pcb =
		*(const physical_compressed_block_cpu *) in;
	symbolic_compressed_block_cpu scb;

	physical_to_symbolic_cpu(BlockWidth, BlockHeight, 1, pcb, &scb);

	swizzlepattern_cpu swz_decode = { 0, 1, 2, 3 };
	imageblock_cpu pb;
	pb.xpos = pb.ypos = pb.zpos = 0;

	decompress_symbolic_block(&scb, &pb, encoder);

	write_imageblock_cpu(
		img, &pb, BlockWidth, BlockHeight, 1, 0, 0, 0, swz_decode);

	// copy results to our output buffer
	CMP_COLOR *ptr = out;
	for (int y = 0; y < BlockHeight; y++)
	{
		memcpy(ptr, img->imagedata8[0][y], BlockWidth * 4);
		ptr += BlockWidth;
	}
}
