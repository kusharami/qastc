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
//  File Name:   CodecBuffer.cpp
//  Description: implementation of the CCodecBuffer class
//
//////////////////////////////////////////////////////////////////////////////

#include "CodecBuffer.h"
#include "CodecBuffer_RGBA8888.h"
#include "CodecBuffer_Block.h"

#include <cassert>
#include <cstdlib>
#include <cstring>

CCodecBuffer *CreateCodecBuffer(CodecBufferType nCodecBufferType,
	CMP_BYTE nBlockWidth, CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth,
	CMP_DWORD dwWidth, CMP_DWORD dwHeight, CMP_DWORD dwPitch, CMP_BYTE *pData)
{
	switch (nCodecBufferType)
	{
		case CBT_RGBA8888:
			return new CCodecBuffer_RGBA8888(dwWidth, dwHeight, dwPitch, pData);

		case CBT_Block:
			return new CCodecBuffer_Block(nBlockWidth, nBlockHeight,
				nBlockDepth, dwWidth, dwHeight, dwPitch, pData);

		case CBT_Unknown:
			break;
	}
	assert(0);
	return nullptr;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCodecBuffer::CCodecBuffer(CodecBufferType type, CMP_BYTE nBlockWidth,
	CMP_BYTE nBlockHeight, CMP_BYTE nBlockDepth, CMP_DWORD dwWidth,
	CMP_DWORD dwHeight, CMP_DWORD dwPitch, CMP_BYTE *pData)
{
	m_nBufferType = type;
	m_dwFormat = CMP_FORMAT_Unknown;
	m_dwWidth = dwWidth;
	m_dwHeight = dwHeight;

	m_nBlockWidth = nBlockWidth;
	m_nBlockHeight = nBlockHeight;
	m_nBlockDepth = nBlockDepth;
	m_dwDepth = nBlockWidth * nBlockHeight * nBlockDepth;
	m_nBlockSize = (m_dwDepth + 7) / 8;

	if (type == CBT_Block)
	{
		m_nColumns = ((dwWidth + nBlockWidth - 1) / nBlockWidth);
		m_nRows = ((dwHeight + nBlockHeight - 1) / nBlockHeight);
	} else
	{
		m_nColumns = m_dwWidth;
		m_nRows = m_dwHeight;
	}

	m_dwPitch = (dwPitch == 0) ? m_nColumns * m_nBlockSize : dwPitch;

	m_pData = pData;
	m_bUserAllocedData = (pData != nullptr);

	if (m_pData == NULL)
	{
		m_pData = (CMP_BYTE *) malloc(GetDataSize());
	}
}

CCodecBuffer::~CCodecBuffer()
{
	if (m_pData && !m_bUserAllocedData)
	{
		free(m_pData);
		m_pData = NULL;
	}
}

void CCodecBuffer::SetBlockDims(CMP_BYTE BlockWidth, CMP_BYTE BlockHeight)
{
	if (m_nBlockWidth == BlockWidth && m_nBlockHeight == BlockHeight)
		return;

	m_nBlockWidth = BlockWidth;
	m_nBlockHeight = BlockHeight;
	if (m_nBufferType == CBT_Block)
	{
		m_nColumns = ((m_dwWidth + BlockWidth - 1) / BlockWidth);
		m_nRows = ((m_dwHeight + BlockHeight - 1) / BlockHeight);
		m_dwPitch = m_nColumns * m_nBlockSize;
		if (!m_bUserAllocedData)
		{
			realloc(m_pData, GetDataSize());
		}
	}
}

void CCodecBuffer::ReadBlock(CMP_DWORD x, CMP_DWORD y, CMP_BYTE *pBlock)
{
	assert(x < GetColumns());
	assert(y < GetRows());

	memcpy(pBlock, m_pData + y * m_dwPitch + x * m_nBlockSize, m_nBlockSize);
}

void CCodecBuffer::WriteBlock(CMP_DWORD x, CMP_DWORD y, CMP_BYTE *pBlock)
{
	assert(x < GetColumns());
	assert(y < GetRows());

	memcpy(m_pData + y * m_dwPitch + x * m_nBlockSize, pBlock, m_nBlockSize);
}

CMP_DWORD CCodecBuffer::GetDataSize() const
{
	return m_dwPitch * m_nRows;
}
