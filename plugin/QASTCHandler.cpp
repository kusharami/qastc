#include "QASTCHandler.h"

#include "ASTC/cASTC.h"
#include "ASTC/Codec_ASTC.h"
#include "Buffer/CodecBuffer_Block.h"

#include <QImage>
#include <QVariant>

struct QASTCHandler::Header
{
	quint8 xdim;
	quint8 ydim;
	quint8 zdim;
	int xsize;
	int ysize;
	int zsize;

	bool readFrom(const astc_header &other);
	bool readFrom(QIODevice *device);
	bool peekFrom(QIODevice *device);

	QByteArray toSubType() const;
	static const QByteArrayList &validSubTypes();
};

QASTCHandler::QASTCHandler()
	: mQuality(-1)
	, mBlockWidth(4)
	, mBlockHeight(4)
{
}

QByteArray QASTCHandler::ASTC_Format()
{
	return QByteArrayLiteral("astc");
}

bool QASTCHandler::validateHeader(QIODevice *device)
{
	if (!device || !device->isReadable())
		return false;

	astc_header header;
	if (device->peek(reinterpret_cast<char *>(&header), sizeof(astc_header)) !=
		sizeof(astc_header))
	{
		return false;
	}

	int magic = header.magic[0] | (header.magic[1] << 8) |
		(header.magic[2] << 16) | (header.magic[3] << 24);

	return magic == MAGIC_FILE_CONSTANT;
}

bool QASTCHandler::canRead() const
{
	if (validateHeader(device()))
	{
		setFormat(ASTC_Format());
		return true;
	}
	return false;
}

bool QASTCHandler::read(QImage *image)
{
	if (!validateHeader(device()))
		return false;

	Header header;
	if (!header.readFrom(device()))
	{
		return false;
	}

	CCodec_ASTC codec;

	QScopedPointer<CCodecBuffer> srcCodecBuffer(codec.CreateBuffer(
		header.xdim, header.ydim, 0, header.xsize, header.ysize));
	auto dwTotalSize = srcCodecBuffer->GetDataSize();
	if (device()->read(reinterpret_cast<char *>(srcCodecBuffer->GetData()),
			dwTotalSize) != dwTotalSize)
	{
		return false;
	}

	QScopedPointer<CCodecBuffer> dstCodecBuffer(
		CreateCodecBuffer(CBT_RGBA8888, 0, 0, 0, header.xsize, header.ysize));

	if (codec.Decompress(*srcCodecBuffer, *dstCodecBuffer) != CE_OK)
	{
		return false;
	}

	auto buffer = dstCodecBuffer.take();
	*image = QImage(buffer->GetData(), header.xsize, header.ysize,
		QImage::Format_RGBA8888, &QASTCHandler::QImageTextureCleanup, buffer);
	return true;
}

bool QASTCHandler::write(const QImage &image)
{
	if (!device() || !device()->isWritable())
		return false;

	CCodec_ASTC codec;
	if (mQuality >= 0)
	{
		codec.setQuality(mQuality / 100.0);
	}
	codec.setBlockRate(mBlockWidth, mBlockHeight);

	auto img = image;
	if (img.format() != QImage::Format_RGBA8888)
		img = img.convertToFormat(QImage::Format_RGBA8888);

	QScopedPointer<CCodecBuffer> srcCodecBuffer(CreateCodecBuffer(CBT_RGBA8888,
		0, 0, 0, img.width(), img.height(), img.bytesPerLine(), img.bits()));

	QScopedPointer<CCodecBuffer> dstCodecBuffer(codec.CreateBuffer(
		mBlockWidth, mBlockHeight, 0, img.width(), img.height()));

	if (codec.Compress(*srcCodecBuffer, *dstCodecBuffer) != CE_OK)
	{
		return false;
	}

	astc_header header;
	header.magic[0] = quint8(MAGIC_FILE_CONSTANT);
	header.magic[1] = quint8(MAGIC_FILE_CONSTANT >> 8);
	header.magic[2] = quint8(MAGIC_FILE_CONSTANT >> 16);
	header.magic[3] = quint8(MAGIC_FILE_CONSTANT >> 24);
	header.blockdim_x = mBlockWidth;
	header.blockdim_y = mBlockHeight;
	header.blockdim_z = 1;
	header.xsize[0] = quint8(img.width());
	header.xsize[1] = quint8(img.width() >> 8);
	header.xsize[2] = quint8(img.width() >> 16);
	header.ysize[0] = quint8(img.height());
	header.ysize[1] = quint8(img.height() >> 8);
	header.ysize[2] = quint8(img.height() >> 16);
	header.zsize[0] = 1;
	header.zsize[1] = 0;
	header.zsize[2] = 0;
	if (device()->write(reinterpret_cast<char *>(&header), sizeof(header)) !=
		sizeof(header))
	{
		return false;
	}
	CMP_DWORD dataSize = dstCodecBuffer->GetDataSize();
	return device()->write(reinterpret_cast<char *>(dstCodecBuffer->GetData()),
			   dataSize) == dataSize;
}

QVariant QASTCHandler::option(ImageOption option) const
{
	switch (option)
	{
		case Size:
		{
			Header header;
			if (header.peekFrom(device()))
				return QSize(header.xsize, header.ysize);
			break;
		}

		case SubType:
		{
			Header header;
			if (header.peekFrom(device()))
				return header.toSubType();
			break;
		}

		case Quality:
			return mQuality;

		case SupportedSubTypes:
		{
			return QVariant::fromValue(Header::validSubTypes());
		}

		case ClipRect:
		case Description:
		case ScaledClipRect:
		case ScaledSize:
		case CompressionRatio:
		case Gamma:
		case Name:
		case IncrementalReading:
		case Endianness:
		case Animation:
		case BackgroundColor:
		case ImageFormat:
		case OptimizedWrite:
		case ProgressiveScanWrite:
		case ImageTransformation:
		case TransformedByDefault:
			break;
	}

	return QVariant();
}

void QASTCHandler::setOption(ImageOption option, const QVariant &value)
{
	switch (option)
	{
		case Quality:
		{
			bool ok;
			int q = value.toInt(&ok);
			mQuality = ok ? q : -1;
			break;
		}

		case SubType:
		{
			auto split = value.toByteArray().split('x');
			if (split.size() == 2)
			{
				int w = split.at(0).toInt();
				int h = split.at(1).toInt();
				if (CCodec_ASTC::isValidBlockSize(w, h, 1))
				{
					mBlockWidth = quint8(w);
					mBlockHeight = quint8(h);
				}
			}

			break;
		}

		case Size:
		case SupportedSubTypes:
		case ClipRect:
		case Description:
		case ScaledClipRect:
		case ScaledSize:
		case CompressionRatio:
		case Gamma:
		case Name:
		case IncrementalReading:
		case Endianness:
		case Animation:
		case BackgroundColor:
		case ImageFormat:
		case OptimizedWrite:
		case ProgressiveScanWrite:
		case ImageTransformation:
		case TransformedByDefault:
			break;
	}
}

bool QASTCHandler::supportsOption(QImageIOHandler::ImageOption option) const
{
	switch (option)
	{
		case Size:
		case Quality:
		case SubType:
		case SupportedSubTypes:
			return true;

		case ClipRect:
		case Description:
		case ScaledClipRect:
		case ScaledSize:
		case CompressionRatio:
		case Gamma:
		case Name:
		case IncrementalReading:
		case Endianness:
		case Animation:
		case BackgroundColor:
		case ImageFormat:
		case OptimizedWrite:
		case ProgressiveScanWrite:
		case ImageTransformation:
		case TransformedByDefault:
			break;
	}

	return false;
}

void QASTCHandler::QImageTextureCleanup(void *ptr)
{
	delete reinterpret_cast<CCodecBuffer *>(ptr);
}

bool QASTCHandler::Header::readFrom(const astc_header &other)
{
	if (!CCodec_ASTC::isValidBlockSize(
			other.blockdim_x, other.blockdim_y, other.blockdim_z))
	{
		return false;
	}

	int xsize = other.xsize[0] + 256 * other.xsize[1] + 65536 * other.xsize[2];
	int ysize = other.ysize[0] + 256 * other.ysize[1] + 65536 * other.ysize[2];
	int zsize = other.zsize[0] + 256 * other.zsize[1] + 65536 * other.zsize[2];

	if (xsize <= 0 || ysize <= 0 || zsize != 1)
	{
		return false;
	}

	xdim = other.blockdim_x;
	ydim = other.blockdim_y;
	zdim = other.blockdim_z;
	this->xsize = xsize;
	this->ysize = ysize;
	this->zsize = zsize;
	return true;
}

bool QASTCHandler::Header::readFrom(QIODevice *device)
{
	if (!device || !device->isReadable())
		return false;

	astc_header header;
	if (device->read(reinterpret_cast<char *>(&header), sizeof(header)) !=
		sizeof(header))
	{
		return false;
	}

	return readFrom(header);
}

bool QASTCHandler::Header::peekFrom(QIODevice *device)
{
	if (!device || !device->isReadable())
		return false;

	device->startTransaction();
	bool ok = readFrom(device);
	device->rollbackTransaction();
	return ok;
}

QByteArray QASTCHandler::Header::toSubType() const
{
	return QByteArray::number(xdim) + "x" + QByteArray::number(ydim);
}

const QByteArrayList &QASTCHandler::Header::validSubTypes()
{
	static QByteArrayList result;

	if (result.isEmpty())
	{
		for (auto &size : ASTC_VALID_BLOCK_SIZE)
		{
			result.append(
				QByteArray::number(size.w) + "x" + QByteArray::number(size.h));
		}
	}

	return result;
}
