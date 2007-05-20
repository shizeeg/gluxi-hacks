#ifndef WWWREQUEST_H
#define WWWREQUEST_H

#include "base/asyncrequest.h"

#include <QString>
#include <QProcess>
#include <QThread>

class WWWRequest: public AsyncRequest
{
	Q_OBJECT
public:
	WWWRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& cmd, const QString& dest);
	~WWWRequest();
	void launch();
private:
	QString myCmd;
	QString myDest;
	QString myExp;
	QProcess *proc;
protected:
	void run();
};

#endif

