#ifndef XEPREQUEST_H
#define XEPREQUEST_H

#include "base/asyncrequest.h"

#include <QString>

class QHttp;

class XepRequest: public AsyncRequest
{
	Q_OBJECT
public:
	XepRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~XepRequest();
	void exec();
private:
	QString myDest;
	QString url;
	QString xep;
	QHttp *http;
private slots:
	void httpRequestFinished(int, bool err);
};

#endif

