#ifndef GOOGLEREQUEST_H
#define GOOGLEREQUEST_H

#include "base/asyncrequest.h"

#include <QString>

class QHttp;

class GoogleRequest: public AsyncRequest
{
	Q_OBJECT
public:
	GoogleRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~GoogleRequest();
	void exec();
private:
	QString myDest;
	QHttp *http;
	int nres;
private slots:
	void httpRequestFinished(int, bool err);
};

#endif

