#ifndef DIRECTPROXY_H
#define DIRECTPROXY_H

#include "baseproxy.h"

#include <QtCore>

class SWInetSocket;

class DirectProxy : public BaseProxy
{
public:
	DirectProxy(int id, int typ, QString nfo);
	QString fetchPage(const QString&host, const QString &url, QString&referer, QStringList *, QStringList*, int custom=0);
	QString fetchPageSSL(const QString&host, const QString &url, QString&referer, QStringList *, QStringList*, int custom=0);
	~DirectProxy();
private:
	int wascustom;
	QByteArray httpRequest(const QString&, const QString&, const QString&, QStringList *, QStringList *, QString&);
	int readResponse(SWInetSocket* socket, MyBuf *buf, QStringList*, QString& redirect);
	int parseResponseLine(QString& line, QStringList*, QString&);
};

#endif
