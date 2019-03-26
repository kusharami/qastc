#include "Tests.h"

#include <QImageReader>
#include <QImageWriter>

#include <QPainter>
#include <QtTest>

struct ASTCTests::Options
{
	int quality;
	QByteArray subType;

	static const Options OPTIONS[];
};

const ASTCTests::Options ASTCTests::Options::OPTIONS[] = {
	{ 100, QByteArrayLiteral("4x4") },
	{ 90, QByteArrayLiteral("5x4") },
	{ -1, QByteArrayLiteral("5x5") },
	{ 4, QByteArrayLiteral("6x5") },
	{ -1, QByteArrayLiteral("6x6") },
	{ -1, QByteArrayLiteral("8x5") },
	{ 0, QByteArrayLiteral("8x6") },
	{ -1, QByteArrayLiteral("10x5") },
	{ 80, QByteArrayLiteral("10x6") },
	{ -1, QByteArrayLiteral("8x8") },
	{ -1, QByteArrayLiteral("10x8") },
	{ -1, QByteArrayLiteral("10x10") },
	{ -1, QByteArrayLiteral("12x10") },
	{ 95, QByteArrayLiteral("12x12") },
};

static const QImage &fetchImage()
{
	static bool imageInitialized = false;

	static QImage image(32, 32, QImage::Format_RGBA8888);
	if (!imageInitialized)
	{
		imageInitialized = true;
		image.fill(Qt::transparent);

		QPainter painter(&image);
		painter.fillRect(QRect(0, 0, 16, 16), QColor(Qt::red));
		painter.fillRect(QRect(16, 0, 16, 16), QColor(Qt::green));
		painter.fillRect(QRect(0, 16, 16, 16), QColor(Qt::blue));
	}

	return image;
}

static bool checkImages(const QImage &a, QImage &b)
{
	return a.pixelColor(8, 8) == b.pixelColor(8, 8) &&
		a.pixelColor(16 + 8, 8) == b.pixelColor(16 + 8, 8) &&
		a.pixelColor(8, 16 + 8) == b.pixelColor(8, 16 + 8) &&
		a.pixelColor(16 + 8, 16 + 8) == b.pixelColor(16 + 8, 16 + 8);
}

template <typename CLASS>
static void checkSupportedOptions(const CLASS &io)
{
	// supported options
	QVERIFY(io.supportsOption(QImageIOHandler::Size));
	QVERIFY(io.supportsOption(QImageIOHandler::Quality));
	QVERIFY(io.supportsOption(QImageIOHandler::SubType));
	QVERIFY(io.supportsOption(QImageIOHandler::SupportedSubTypes));
	// unsupported options
	QVERIFY(!io.supportsOption(QImageIOHandler::ScaledSize));
	QVERIFY(!io.supportsOption(QImageIOHandler::ImageFormat));
	QVERIFY(!io.supportsOption(QImageIOHandler::ImageTransformation));
	QVERIFY(!io.supportsOption(QImageIOHandler::Gamma));
	QVERIFY(!io.supportsOption(QImageIOHandler::Animation));
	QVERIFY(!io.supportsOption(QImageIOHandler::IncrementalReading));
	QVERIFY(!io.supportsOption(QImageIOHandler::Endianness));
	QVERIFY(!io.supportsOption(QImageIOHandler::BackgroundColor));
	QVERIFY(!io.supportsOption(QImageIOHandler::OptimizedWrite));
	QVERIFY(!io.supportsOption(QImageIOHandler::ProgressiveScanWrite));
	QVERIFY(!io.supportsOption(QImageIOHandler::Name));
	QVERIFY(!io.supportsOption(QImageIOHandler::Description));
	QVERIFY(!io.supportsOption(QImageIOHandler::ClipRect));
	QVERIFY(!io.supportsOption(QImageIOHandler::ScaledClipRect));
}

void ASTCTests::testInstallation()
{
	QList<QList<QByteArray>> supported;
	supported.append(QImageReader::supportedImageFormats());
	supported.append(QImageWriter::supportedImageFormats());

	for (auto &s : supported)
	{
		QVERIFY(s.indexOf("astc") >= 0);
	}
}

void ASTCTests::testIO()
{
	QTemporaryDir tempDir;
	tempDir.setAutoRemove(true);
	QVERIFY(tempDir.isValid());

	QDir dir(tempDir.path());

	for (const Options &options : Options::OPTIONS)
	{
		testWrite(options, dir);
		testRead(options, dir);
	}
}

void ASTCTests::testWrite(const Options &options, const QDir &dir)
{
	QImageWriter writer;

	auto &subType = options.subType;

	writer.setFileName(filePathForSubType(dir, subType));
	QVERIFY(writer.canWrite());
	checkSupportedOptions(writer);

	auto supportedSubTypes = writer.supportedSubTypes();

	QVERIFY(supportedSubTypes.indexOf(subType) >= 0);

	writer.setQuality(options.quality);
	writer.setSubType(subType);

	auto &image = fetchImage();
	QVERIFY(writer.write(image));
}

void ASTCTests::testRead(const ASTCTests::Options &options, const QDir &dir)
{
	auto &image = fetchImage();

	QImageReader reader;

	auto &subType = options.subType;

	reader.setFileName(filePathForSubType(dir, subType));
	QVERIFY(reader.canRead());
	checkSupportedOptions(reader);

#if QT_VERSION >= QT_VERSION_CHECK(5, 9, 1)
	// QImageReader::supportedSubTypes always empty in previous versions
	{
		QImageWriter writer;
		writer.setFormat(QByteArrayLiteral("astc"));
		QCOMPARE(reader.supportedSubTypes(), writer.supportedSubTypes());
	}
#endif
	QCOMPARE(reader.size(), image.size());
	QCOMPARE(reader.subType(), subType);

	QImage readImage;

	QVERIFY(reader.read(&readImage));

	QCOMPARE(readImage.size(), image.size());
	QCOMPARE(readImage.format(), QImage::Format_RGBA8888);
	QVERIFY(checkImages(image, readImage));
}

QString ASTCTests::filePathForSubType(
	const QDir &dir, const QByteArray &subType)
{
	return dir.filePath(subType + QByteArrayLiteral(".astc"));
}
