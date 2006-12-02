#ifndef TRACEROUTEREQUEST_H
#define TRACEROUTEREQUEST_H

#include "base/asyncrequest.h"

#include <QString>
#include <QProcess>

class TraceRouteRequest: public AsyncRequest
{
	Q_OBJECT
public:
	TraceRouteRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~TraceRouteRequest();
	void exec();
private:
	QString myDest;
	QProcess *proc;
private slots:
	void onProcessFinished();
	void onStateChanged( QProcess::ProcessState newState );
};

#endif

