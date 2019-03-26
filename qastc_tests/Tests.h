#pragma once

#include <QObject>

class QDir;
class ASTCTests : public QObject
{
	Q_OBJECT

private slots:
	void testInstallation();
	void testIO();

private:
	struct Options;

	void testWrite(const Options &options, const QDir &dir);
	void testRead(const Options &options, const QDir &dir);
	QString filePathForSubType(const QDir &dir, const QByteArray &subType);
};
