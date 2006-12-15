#ifndef SVNREQUEST_H
#define SVNREQUEST_H

#include "base/asyncrequest.h"

#include <QString>
#include <QProcess>

class SVNRequest: public AsyncRequest
{
	Q_OBJECT
public:
	SVNRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~SVNRequest();
	void exec();
private:
	QString myDest;
	QProcess *proc;
private slots:
	void onProcessFinished();
	void onStateChanged( QProcess::ProcessState newState );
};

#endif

