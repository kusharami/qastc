#include "QASTCPlugin.h"

#include "QASTCHandler.h"

QImageIOPlugin::Capabilities QASTCPlugin::capabilities(
	QIODevice *device, const QByteArray &format) const
{
	if (format == QASTCHandler::ASTC_Format())
	{
		return Capabilities(CanRead | CanWrite);
	}

	if (!format.isEmpty())
		return 0;

	if (nullptr == device || !device->isOpen())
		return 0;

	Capabilities result;

	if (QASTCHandler::validateHeader(device))
	{
		result |= CanRead;
	}

	if (device->isWritable())
	{
		result |= CanWrite;
	}

	return result;
}

QImageIOHandler *QASTCPlugin::create(
	QIODevice *device, const QByteArray &format) const
{
	auto handler = new QASTCHandler;
	handler->setDevice(device);
	if (!format.isEmpty())
		handler->setFormat(format);

	return handler;
}
