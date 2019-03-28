//===============================================================================
// Copyright (c) 2007-2017  Advanced Micro Devices, Inc. All rights reserved.
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
//
// ASTC_Encode.cpp : A reference encoder for ASTC
//

#include "ASTC_Encode.h"
#include "ASTC_Encode_Kernel.h"
#include "ASTC_Host.h"

void ASTCBlockEncoder::CompressBlock_kernel(astc_codec_image *input_image,
	uint8_t *bp, int x, int y, ASTC_Encoder::ASTC_Encode *ASTCEncode,
	ASTC_Encoder::compress_symbolic_block_buffers *buffers)
{
	imageblock_cpu m_pb;
	symbolic_compressed_block scb;

	fetch_imageblock_cpu(input_image, &m_pb, x, y, 0, ASTCEncode);

	ASTC_Encoder::compress_symbolic_block(&m_pb, &scb, ASTCEncode, buffers);
	physical_compressed_block pcb;
	pcb = ASTC_Encoder::symbolic_to_physical(&scb, ASTCEncode);

	*(physical_compressed_block *) bp = pcb;
}
