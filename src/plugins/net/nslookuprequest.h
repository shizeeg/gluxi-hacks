#ifndef NSLOOKUPREQUEST_H
#define NSLOOKUPREQUEST_H

#include "base/asyncrequest.h"

#include <QString>
#include <QProcess>

class NslookupRequest: public AsyncRequest
{
	Q_OBJECT
public:
	NslookupRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~NslookupRequest();
	void exec();
private:
	QString myDest;
	QProcess *proc;
private slots:
	void onProcessFinished();
	void onStateChanged( QProcess::ProcessState newState );
};

#endif

