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
//  File Name:   CodecBuffer.h
//  Description: interface for the CCodecBuffer class
//
//////////////////////////////////////////////////////////////////////////////

#ifndef _CODECBUFFER_H_INCLUDED_
#define _CODECBUFFER_H_INCLUDED_

#include "CommonTypes.h"
#include "MathMacros.h"

typedef enum _CodecBufferType {
	CBT_Unknown = 0,
	CBT_RGBA8888,
	CBT_Block
} CodecBufferType;

class CCodecBuffer
{
public:
	CCodecBuffer(CodecBufferType type, CMP_BYTE nBlockWidth,
		CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth, CMP_DWORD dwWidth,
		CMP_DWORD dwHeight, CMP_DWORD dwPitch = 0, CMP_BYTE *pData = 0);
	virtual ~CCodecBuffer();

	inline CodecBufferType GetBufferType() const;

	inline CMP_DWORD GetWidth() const;
	inline CMP_DWORD GetHeight() const;
	inline CMP_WORD GetDepth() const;

	inline CMP_DWORD GetColumns() const;
	inline CMP_DWORD GetRows() const;

	inline CMP_DWORD GetPitch() const;
	inline void SetPitch(CMP_DWORD dwPitch);

	inline void SetFormat(CMP_FORMAT dwFormat);
	inline CMP_FORMAT GetFormat() const;

	inline CMP_BYTE GetBlockWidth() const;
	inline CMP_BYTE GetBlockHeight() const;
	inline CMP_BYTE GetBlockDepth() const;
	inline CMP_WORD GetBlockSize() const;

	void SetBlockDims(CMP_BYTE BlockWidth, CMP_BYTE BlockHeight);
	inline void SetBlockDepth(CMP_BYTE BlockDepth);
	inline void SetBlockSize(CMP_WORD BlockSize);

	void ReadBlock(CMP_DWORD x, CMP_DWORD y, CMP_BYTE *pBlock);
	void WriteBlock(CMP_DWORD x, CMP_DWORD y, CMP_BYTE *pBlock);

	inline CMP_BYTE *GetData() const;
	CMP_DWORD GetDataSize() const;

protected:
	CMP_DWORD m_dwPitch;
	CMP_DWORD m_dwWidth; // Final Image Width
	CMP_DWORD m_dwHeight; // Final Image Height
	CMP_WORD m_dwDepth; // Final Image Depth
	CMP_FORMAT m_dwFormat;
	CodecBufferType m_nBufferType;

	CMP_DWORD m_nColumns;
	CMP_DWORD m_nRows;
	CMP_WORD m_nBlockSize;
	CMP_BYTE m_nBlockWidth; // DeCompression Block Sizes (Default is 4x4x1)
	CMP_BYTE m_nBlockHeight; //
	CMP_BYTE m_nBlockDepth; //

	bool m_bUserAllocedData;
	CMP_BYTE *m_pData;
};

CodecBufferType CCodecBuffer::GetBufferType() const
{
	return m_nBufferType;
}

CMP_DWORD CCodecBuffer::GetWidth() const
{
	return m_dwWidth;
}

CMP_DWORD CCodecBuffer::GetHeight() const
{
	return m_dwHeight;
}

CMP_WORD CCodecBuffer::GetDepth() const
{
	return m_dwDepth;
}

CMP_DWORD CCodecBuffer::GetColumns() const
{
	return m_nColumns;
}

CMP_DWORD CCodecBuffer::GetRows() const
{
	return m_nRows;
}

CMP_DWORD CCodecBuffer::GetPitch() const
{
	return m_dwPitch;
}

void CCodecBuffer::SetPitch(CMP_DWORD dwPitch)
{
	m_dwPitch = dwPitch;
}

void CCodecBuffer::SetFormat(CMP_FORMAT dwFormat)
{
	m_dwFormat = dwFormat;
}

CMP_FORMAT CCodecBuffer::GetFormat() const
{
	return m_dwFormat;
}

CMP_BYTE CCodecBuffer::GetBlockWidth() const
{
	return m_nBlockWidth;
}

CMP_BYTE CCodecBuffer::GetBlockHeight() const
{
	return m_nBlockHeight;
}

CMP_BYTE CCodecBuffer::GetBlockDepth() const
{
	return m_nBlockDepth;
}

CMP_WORD CCodecBuffer::GetBlockSize() const
{
	return m_nBlockSize;
}

void CCodecBuffer::SetBlockDepth(CMP_BYTE BlockDepth)
{
	m_nBlockDepth = BlockDepth;
}

void CCodecBuffer::SetBlockSize(CMP_WORD BlockSize)
{
	m_nBlockSize = BlockSize;
}

CMP_BYTE *CCodecBuffer::GetData() const
{
	return m_pData;
}

CCodecBuffer *CreateCodecBuffer(CodecBufferType nCodecBufferType,
	CMP_BYTE nBlockWidth, CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth,
	CMP_DWORD dwWidth, CMP_DWORD dwHeight, CMP_DWORD dwPitch = 0,
	CMP_BYTE *pData = 0);

#endif // !defined(_CODECBUFFER_H_INCLUDED_)
