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
//
//  File Name:   Codec_ASTC.cpp
//  Description: implementation of the CCodec_ASTC class
//
//////////////////////////////////////////////////////////////////////////////

#include "Codec_ASTC.h"

#include "ASTC_Host.h"
#include "ARM/astc_codec_internals.h"
#include "Buffer/CodecBuffer.h"
#include "MathMacros.h"

#include <cassert>
#include <atomic>
#include <thread>
#include <vector>
#include <memory>

CMP_BYTE CCodec_ASTC::sDefaultEncodeThreads =
	CMP_BYTE(std::thread::hardware_concurrency());

const astc_block_size_t ASTC_VALID_BLOCK_SIZE[ASTC_VALID_BLOCK] = {
	{ 4, 4 }, //
	{ 5, 4 }, //
	{ 5, 5 }, //
	{ 6, 5 }, //
	{ 6, 6 }, //
	{ 8, 5 }, //
	{ 8, 6 }, //
	{ 10, 5 }, //
	{ 10, 6 }, //
	{ 8, 8 }, //
	{ 10, 8 }, //
	{ 10, 10 }, //
	{ 12, 10 }, //
	{ 12, 12 }, //
};

//======================================================================================
struct ASTCEncodeBlockData
{
	// Encoder params
	ASTC_Encoder::compress_symbolic_block_buffers *buffers;
	astc_codec_image *input_image;
	CMP_BYTE *bp;
	int x;
	int y;

	void encode(ASTC_Encoder::ASTC_Encode *encoder);
};

struct ASTCEncodeThread
{
	std::vector<ASTCEncodeBlockData> blocks;
	ASTC_Encoder::compress_symbolic_block_buffers buffers;

	ASTCEncodeThread(ASTC_Encoder::ASTC_Encode *encoder);
	~ASTCEncodeThread();

	void start();

private:
	ASTC_Encoder::ASTC_Encode *encoder;
	std::thread thread;
	void work();
};

//////////////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////////

CCodec_ASTC::CCodec_ASTC()
{
	m_NumThreads = sDefaultEncodeThreads;
	m_xdim = 4;
	m_ydim = 4;
	m_Quality = 0.5;
}

CMP_BYTE CCodec_ASTC::getDefaultEncodeThreads()
{
	return sDefaultEncodeThreads;
}

void CCodec_ASTC::setDefaultEncodeThreads(CMP_BYTE value)
{
	sDefaultEncodeThreads = value;
}

bool CCodec_ASTC::isValidBlockSize(int w, int h, int d)
{
	if (d == 1)
	{
		for (auto &size : ASTC_VALID_BLOCK_SIZE)
		{
			if (w == size.w && h == size.h)
				return true;
		}
	}
	return false;
}

bool CCodec_ASTC::setBlockRate(int x, int y)
{
	if (isValidBlockSize(x, y))
	{
		m_xdim = x;
		m_ydim = y;
		return true;
	}
	return false;
}

CodecError CCodec_ASTC::Compress(
	CCodecBuffer &bufferIn, CCodecBuffer &bufferOut)
{
	if (bufferIn.GetBufferType() != CBT_RGBA8888)
	{
		printf("Unsupported type of input buffer\n");
		return CE_Unknown;
	}

	if (bufferOut.GetFormat() != CMP_FORMAT_ASTC)
	{
		printf("Unsupported type of output buffer\n");
		return CE_Unknown;
	}

	int xsize = bufferIn.GetWidth();
	int ysize = bufferIn.GetHeight();
	m_xdim = bufferOut.GetBlockWidth();
	m_ydim = bufferOut.GetBlockHeight();

	astc_codec_image_cpu *input_image =
		allocate_image_cpu(8, xsize, ysize, 1, 0);

	if (!input_image)
	{
		printf("Unable to allocate image buffer\n");
		return CE_Unknown;
	}

	auto pData = bufferIn.GetData();
	for (int y = ysize - 1; y >= 0; y--)
	{
		memcpy(input_image->imagedata8[0][y], pData, bufferIn.GetPitch());
		pData += bufferIn.GetPitch();
	}

	int xdim = m_xdim;
	int ydim = m_ydim;
	CMP_BYTE *bufferOutput = bufferOut.GetData();

	int xblocks = bufferOut.GetColumns();
	int yblocks = bufferOut.GetRows();

	std::unique_ptr<ASTC_Encoder::ASTC_Encode> encoder(
		new ASTC_Encoder::ASTC_Encode);
	encoder->m_decode_mode = ASTC_DECODE_HDR;
	encoder->m_rgb_force_use_of_hdr = 0;
	encoder->m_alpha_force_use_of_hdr = 0;
	encoder->m_perform_srgb_transform = 0;
	encoder->m_Quality = (float) m_Quality;
	encoder->m_xdim = m_xdim;
	encoder->m_ydim = m_ydim;
	encoder->m_zdim = 1;
	ASTC_Encoder::init_ASTC(encoder.get());
	{
		// setup compression threads for each
		// block to encode  we will load the buffer to pass to ASTC code as 8 bit 4x4 blocks
		// the fill in source image. ASTC code will then use the adaptive sizes for process on the input
		CMP_WORD numEncodingThreads =
			MIN(m_NumThreads, CMP_WORD(MAX_ASTC_THREADS));
		if (numEncodingThreads == 0)
			numEncodingThreads = 1;
		std::vector<std::unique_ptr<ASTCEncodeThread>> threads;
		size_t blocksPerThread = 0;
		std::unique_ptr<ASTC_Encoder::compress_symbolic_block_buffers> buffers;
		if (numEncodingThreads > 1)
		{
			threads.reserve(numEncodingThreads);
			blocksPerThread = (xblocks * yblocks + numEncodingThreads - 1) /
				numEncodingThreads;
		} else
		{
			buffers.reset(new ASTC_Encoder::compress_symbolic_block_buffers);
		}

		size_t threadIndex = 0;
		size_t counter = 0;
		ASTCEncodeBlockData blockData;
		for (int y = 0; y < yblocks; y++)
		{
			int yoffset = y * xblocks;
			for (int x = 0; x < xblocks; x++)
			{
				int offset = (yoffset + x) * ASTC_COMPRESSED_BLOCK_SIZE;
				CMP_BYTE *bp = bufferOutput + offset;

				blockData.input_image = input_image;
				blockData.bp = bp;
				blockData.x = x * xdim;
				blockData.y = y * ydim;

				if (numEncodingThreads > 1)
				{
					if (counter == 0)
					{
						auto thread = new ASTCEncodeThread(encoder.get());
						threads.emplace_back(thread);
						thread->blocks.reserve(blocksPerThread);
					}
					auto thread = threads.at(threadIndex).get();
					blockData.buffers = &thread->buffers;
					thread->blocks.emplace_back(blockData);

					if (++counter == blocksPerThread)
					{
						counter = 0;
						threadIndex++;
					}
				} else
				{
					blockData.buffers = buffers.get();
					blockData.encode(encoder.get());
				}
			}
		}

		for (auto &thread : threads)
		{
			thread->start();
		}
	} // all threads join here

	destroy_image_cpu(input_image);

	return CE_OK;
}

CodecError CCodec_ASTC::Decompress(
	CCodecBuffer &bufferIn, CCodecBuffer &bufferOut)
{
	if (bufferIn.GetFormat() != CMP_FORMAT_ASTC)
	{
		printf("Unsupported type of input buffer\n");
		return CE_Unknown;
	}

	if (bufferOut.GetBufferType() != CBT_RGBA8888)
	{
		printf("Unsupported type of output buffer\n");
		return CE_Unknown;
	}

	CMP_BYTE Block_Width = bufferIn.GetBlockWidth();
	CMP_BYTE Block_Height = bufferIn.GetBlockHeight();
	m_xdim = Block_Width;
	m_ydim = Block_Height;

	std::unique_ptr<ASTC_Encoder::ASTC_Encode> codec(
		new ASTC_Encoder::ASTC_Encode);
	codec->m_decode_mode = ASTC_DECODE_HDR;
	codec->m_rgb_force_use_of_hdr = 0;
	codec->m_alpha_force_use_of_hdr = 0;
	codec->m_perform_srgb_transform = 0;
	codec->m_xdim = m_xdim;
	codec->m_ydim = m_ydim;
	codec->m_zdim = 1;
	codec->m_Quality = 0.f;
	ASTC_Encoder::init_ASTC(codec.get());

	ASTCBlockDecoder decoder(codec.get(), Block_Width, Block_Height);

	const CMP_DWORD imageWidth = bufferIn.GetWidth();
	const CMP_DWORD imageHeight = bufferIn.GetHeight();

	const CMP_DWORD dwBlocksX = bufferIn.GetColumns();
	const CMP_DWORD dwBlocksY = bufferIn.GetRows();

	// Output data size Pitch
	CMP_DWORD dwPitch = bufferOut.GetPitch();

	// Output Buffer
	CMP_BYTE *pDataOut = bufferOut.GetData();

	CMP_COLOR DecData[ASTC_MAX_BLOCK_SIZE * ASTC_MAX_BLOCK_SIZE];
	CMP_BYTE CompData[ASTC_COMPRESSED_BLOCK_SIZE];
	for (CMP_DWORD cmpRowY = 0; cmpRowY < dwBlocksY; cmpRowY++)
	{
		for (CMP_DWORD cmpColX = 0; cmpColX < dwBlocksX; cmpColX++)
		{
			bufferIn.ReadBlock(cmpColX, cmpRowY, CompData);
			// Decode to the appropriate location in the compressed image
			decoder.decompress(DecData, CompData);

			// Now that we have a decoded block lets copy that
			// data over to the target image buffer
			CMP_DWORD outX = cmpColX * Block_Width;
			CMP_DWORD outY = cmpRowY * Block_Height;

			for (int row = 0; row < Block_Height; row++)
			{
				CMP_DWORD h = outY + row;
				if (h >= imageHeight)
					break;

				CMP_DWORD nextRowCol =
					(imageHeight - 1 - h) * dwPitch + (outX * 4);
				auto pData =
					reinterpret_cast<CMP_COLOR *>(pDataOut + nextRowCol);
				int yoffset = row * Block_Width;
				for (int col = 0; col < Block_Width; col++)
				{
					CMP_DWORD w = outX + col;
					if (w >= imageWidth)
						break;

					*pData++ = DecData[yoffset + col];
				}
			}
		}
	}

	return CE_OK;
}

CCodecBuffer *CCodec_ASTC::CreateBuffer(CMP_BYTE nBlockWidth,
	CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth, CMP_DWORD dwWidth,
	CMP_DWORD dwHeight, CMP_DWORD dwPitch, CMP_BYTE *pData) const
{
	assert(nBlockDepth == 0);
	assert(dwPitch == 0);
	auto buffer =
		CreateCodecBuffer(CBT_Block, 4, 4, 8, dwWidth, dwHeight, 0, pData);
	buffer->SetFormat(CMP_FORMAT_ASTC);
	buffer->SetBlockDims(nBlockWidth, nBlockHeight);
	assert(buffer->GetBlockSize() == ASTC_COMPRESSED_BLOCK_SIZE);
	assert(buffer->GetPitch() ==
		ASTC_COMPRESSED_BLOCK_SIZE * buffer->GetColumns());
	return buffer;
}

ASTCEncodeThread::ASTCEncodeThread(ASTC_Encoder::ASTC_Encode *encoder)
	: encoder(encoder)
{
}

ASTCEncodeThread::~ASTCEncodeThread()
{
	if (thread.joinable())
		thread.join();
}

void ASTCEncodeThread::start()
{
	if (blocks.empty())
		return;

	assert(!thread.joinable());
	thread = std::thread(&ASTCEncodeThread::work, this);
}

void ASTCEncodeThread::work()
{
	for (auto &block : blocks)
	{
		block.encode(encoder);
	}
}

void ASTCEncodeBlockData::encode(ASTC_Encoder::ASTC_Encode *encoder)
{
	ASTCBlockEncoder::CompressBlock_kernel(
		input_image, bp, x, y, encoder, buffers);
}
