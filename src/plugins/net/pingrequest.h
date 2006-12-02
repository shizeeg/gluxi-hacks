#ifndef PINGREQUEST_H
#define PINGREQUEST_H

#include "base/asyncrequest.h"

#include <QString>

class QProcess;

class PingRequest: public AsyncRequest
{
	Q_OBJECT
public:
	PingRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~PingRequest();
	void exec();
private:
	QString myDest;
	QProcess *proc;
};

#endif

