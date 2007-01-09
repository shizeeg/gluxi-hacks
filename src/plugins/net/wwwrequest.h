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
	WWWRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~WWWRequest();
	void launch();
private:
	QString myDest;
	QProcess *proc;
protected:
	void run();
};

#endif

