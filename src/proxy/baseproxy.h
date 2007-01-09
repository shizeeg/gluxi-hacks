#ifndef BASEPROXY_H
#define BASEPROXY_H

#include <QString>
#include <QTextStream>
#include <QStringList>

#define MYBUFSIZE 16384
struct MyBuf
{
	char data[MYBUFSIZE];
	int count;
	int pos;
};

class BaseProxy
{
public:
	BaseProxy(int id, int typ, QString nfo);
	virtual ~BaseProxy();

	QString getInfo() const { return info;};
	int getType() const { return type;};
	int getUsage() const { return usage;};
	int getID() const {return id;};
	void addClient() { usage++;};
	void removeClient() { usage-- ;};
	void kill();
	virtual QString fetchPage(const QString&host, const QString& url, QString& referer, 
		QStringList *, QStringList*, int custom=0 );
	virtual QString fetchPageSSL(const QString&host, const QString& url, QString& referer, 
		QStringList *, QStringList*, int custom=0);
	virtual QString fetch(const QString& url, QString& referer, QStringList*, QStringList*, int custom=0);
	QTextStream stream;
	QByteArray lastData;
	QByteArray postData;
	int postSize;
	int fails;
	int timeout;
	int custom;
	int maxSize;
	QStringList contentTypes;
	QString contentType;
	QString charset;
	QStringList headers;
	int contentLength;
	bool headersOnly;
	QString errorString;
private:
	int id;
	int type;
	QString info;
	int usage;
protected:
	QString proxyIP;
	int proxyPort;
};

#endif
