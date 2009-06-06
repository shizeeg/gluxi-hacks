#ifndef TRANSLATEREQUEST_H
#define TRANSLATEREQUEST_H

#include "base/asyncrequest.h"

#include <QString>

class QHttp;
class MessageParser;

class TranslateRequest: public AsyncRequest
{
	Q_OBJECT
public:
	TranslateRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser);
	~TranslateRequest();
	void exec();
private:
	QString myType;
	QString myFrom;
	QString myTo;
	QString myText;
	QHttp *http;
	int nres;
private slots:
	void httpRequestFinished(int, bool err);
	void httpRequestListFinished(int, bool err);

	void httpRequestYandexFinished(int, bool err);
	void httpRequestListYandexFinished(int, bool err);
};

#endif

