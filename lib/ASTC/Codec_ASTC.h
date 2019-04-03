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
//////////////////////////////////////////////////////////////////////////////////

#if !defined(_CODEC_ASTC_H_INCLUDED_)
#define _CODEC_ASTC_H_INCLUDED_

#include "Codec.h"
#include "ASTC_Encode.h"
#include "ASTC_Decode.h"
#include "ASTC_Definitions.h"
#include "ASTC_Host.h"

struct astc_block_size_t
{
	CMP_BYTE w;
	CMP_BYTE h;
};

enum
{
	ASTC_VALID_BLOCK = 14
};
extern const astc_block_size_t ASTC_VALID_BLOCK_SIZE[ASTC_VALID_BLOCK];

class CCodec_ASTC : public CCodec
{
public:
	CCodec_ASTC();

	static CMP_BYTE getDefaultEncodeThreads();
	static void setDefaultEncodeThreads(CMP_BYTE value);

	static bool isValidBlockSize(int w, int h, int d = 1);

	inline CMP_WORD getNumThreads() const;
	inline void setNumThreads(CMP_WORD value);

	inline int getBlockRateX() const;
	inline int getBlockRateY() const;
	bool setBlockRate(int x, int y);

	inline double getQuality() const;
	inline void setQuality(double value);

	// Required interfaces
	virtual CodecError Compress(
		CCodecBuffer &bufferIn, CCodecBuffer &bufferOut);
	virtual CodecError Decompress(
		CCodecBuffer &bufferIn, CCodecBuffer &bufferOut);

	virtual CCodecBuffer *CreateBuffer(CMP_BYTE nBlockWidth,
		CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth, CMP_DWORD dwWidth,
		CMP_DWORD dwHeight, CMP_DWORD dwPitch = 0, CMP_BYTE *pData = 0) const;

private:
	static CMP_BYTE sMaxEncodeThreads;
	static CMP_BYTE sDefaultEncodeThreads;

	CMP_WORD m_NumThreads;

	int m_xdim, m_ydim;

	double m_Quality;
};

CMP_WORD CCodec_ASTC::getNumThreads() const
{
	return m_NumThreads;
}

void CCodec_ASTC::setNumThreads(CMP_WORD value)
{
	m_NumThreads = value;
}

int CCodec_ASTC::getBlockRateX() const
{
	return m_xdim;
}

int CCodec_ASTC::getBlockRateY() const
{
	return m_ydim;
}

double CCodec_ASTC::getQuality() const
{
	return m_Quality;
}

void CCodec_ASTC::setQuality(double value)
{
	m_Quality = value;
}

#endif // !defined(_CODEC_ASTC_H_INCLUDED_)
