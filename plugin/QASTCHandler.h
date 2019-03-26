#pragma once

#include <QImageIOHandler>

class QASTCHandler : public QImageIOHandler
{
	struct Header;
	int mQuality;
	quint8 mBlockWidth;
	quint8 mBlockHeight;

public:
	QASTCHandler();

	static QByteArray ASTC_Format();

	static bool validateHeader(QIODevice *device);

	virtual bool canRead() const override;
	virtual bool read(QImage *image) override;
	virtual bool write(const QImage &image) override;

	virtual QVariant option(ImageOption option) const override;
	virtual void setOption(ImageOption option, const QVariant &value) override;
	virtual bool supportsOption(ImageOption option) const override;

private:
	static void QImageTextureCleanup(void *ptr);
};
