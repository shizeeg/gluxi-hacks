#ifndef WWWREQUEST_H
#define WWWREQUEST_H

#include "base/asyncrequest.h"

#include <QString>
#include <QProcess>

class WWWRequest: public AsyncRequest
{
	Q_OBJECT
public:
	WWWRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~WWWRequest();
	void exec();
private:
	QString myDest;
	QProcess *proc;
private slots:
	void onProcessFinished();
	void onStateChanged( QProcess::ProcessState newState );
};

#endif

