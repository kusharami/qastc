//===============================================================================
// Copyright (c) 2014-2016  Advanced Micro Devices, Inc. All rights reserved.
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
//////////////////////////////////////////////////////////////////////////////////

#ifndef _ASTC_DECODE_H_
#define _ASTC_DECODE_H_

#include "ASTC_Definitions.h"
#include "ASTC_Encode_Kernel.h"
#include "ASTC_Host.h"
#include "CommonTypes.h"

class ASTCBlockDecoder
{
	ASTC_Encoder::ASTC_Encode *codec;
	astc_codec_image_cpu *img;

public:
	ASTCBlockDecoder(
		ASTC_Encoder::ASTC_Encode *codec, BYTE BlockWidth, BYTE BlockHeight);
	~ASTCBlockDecoder();

	void decompress(CMP_COLOR out[], const BYTE in[ASTC_COMPRESSED_BLOCK_SIZE]);

	static void DecompressBlock(BYTE BlockWidth, BYTE BlockHeight,
		CMP_COLOR out[], const BYTE in[ASTC_COMPRESSED_BLOCK_SIZE],
		ASTC_Encoder::ASTC_Encode *codec);

	static void DecompressBlock(astc_codec_image_cpu *img, CMP_COLOR out[],
		const BYTE in[ASTC_COMPRESSED_BLOCK_SIZE],
		ASTC_Encoder::ASTC_Encode *codec);
};

#endif
