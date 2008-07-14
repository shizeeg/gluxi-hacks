#include "asyncrequest.h"

#include <gloox/stanza.h>

#include <QtDebug>
#include <QTimer>

AsyncRequest::AsyncRequest(int id, BasePlugin *plugin, gloox::Stanza *from, int timeout, bool notifyOnTimeout)
{
	myId=id;
	myPlugin=plugin;
	myStanza=from;
	myTimeout=timeout;
	notifyOnTimeout_=notifyOnTimeout;
	update();
	connect(this, SIGNAL(terminated()), SLOT(sltThreadTerminated()));
	connect(this, SIGNAL(finished()), SLOT(sltThreadFinished()));
	if (notifyOnTimeout)
		QTimer::singleShot(timeout*1000, this, SLOT(sltTimerTimeout()));
}

AsyncRequest::~AsyncRequest()
{
	qDebug() << "~AsyncRequest";
	if (myStanza)
		delete myStanza;
	emit onDelete(this);
}

void AsyncRequest::update()
{
	myTime=QDateTime::currentDateTime();
	notified=false;
}

bool AsyncRequest::expired()
{
	bool res=myTime.secsTo(QDateTime::currentDateTime())>myTimeout;
	if (res && !notified)
	{
		emit onExpire();
		notified=true;
	}
	return res;
}

QString AsyncRequest::stanzaId() const
{
        if (!myStanza)
                return QString::null;
        return QString::fromStdString(myStanza->findAttribute("id"));
}

void AsyncRequest::run()
{
}

void AsyncRequest::sltThreadTerminated()
{
	emit onTerminated(this);
}

void AsyncRequest::sltThreadFinished()
{
	emit onFinished(this);
}

void AsyncRequest::sltTimerTimeout()
{
	emit onTimeout(this);
	emit onFinished(this);
}
