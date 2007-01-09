#include "baseproxy.h"

#include <QtDebug>

BaseProxy::BaseProxy(int ID, int typ, QString nfo)
{
	id=ID;
	info=nfo;
	type=typ;
	fails=0;
	usage=0;
	proxyIP=nfo.section(":",0,0);
	proxyPort=nfo.section(":",1,1).toInt();
	timeout=30;
	custom=0;
	postSize=0;
	maxSize=0;
	headersOnly=false;
}

BaseProxy::~BaseProxy()
{}

void BaseProxy::kill()
{
	qDebug("Killing BaseProxy");
}

QString BaseProxy::fetchPage(const QString&, const QString &, QString&, QStringList *, QStringList*, int)
{
	qDebug() <<"Base proxy can't fetch pages";
	return QString::null;
}

QString BaseProxy::fetchPageSSL(const QString&, const QString &, QString&, QStringList *, QStringList*, int)
{
	qDebug() <<"Base proxy can't fetch pages";
	return QString::null;
}

QString BaseProxy::fetch(const QString& url, QString& referer, QStringList*a, QStringList*b, int cust)
{
	if (cust)
	{
		printf("BASEPROXY: USING CUSTOM REQUEST\n");
	}
	QString proto=url.section("//",0,0).toUpper();
	if (proto!="HTTP:" && proto !="HTTPS:")
	{	
		errorString="Protocol not supported";
		return QString::null;
	}
	QString s=url.section("//",1);
	QString serv=s.section("/",0,0);
	QString doc="/"+s.section("/",1);
	if (serv.isEmpty()) return QString::null;
	QString res;
	if (proto=="HTTP:")
		res=fetchPage(serv,doc,referer,a,b, cust);
	else
		if (proto=="HTTPS:")
			res=fetchPageSSL(serv,doc,referer,a,b, cust);
		else
		{
			res=QString::null;
			qDebug() << "Unsupported protocol: " << proto;
		}
	referer=url;
	return res;
}
