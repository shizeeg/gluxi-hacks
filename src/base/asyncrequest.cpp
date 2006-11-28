#include "asyncrequest.h"

#include <gloox/stanza.h>

AsyncRequest::AsyncRequest(int timeout)
{
	myPlugin=0L;
	myStanza=0L;
	mySource=0L;
	myName="";
	myTimeout=timeout;
	update();
}

AsyncRequest::AsyncRequest(BasePlugin *plugin, const QString& name, gloox::Stanza *stanza, int timeout)
{
	myPlugin=plugin;
	myName=name;
	myStanza=stanza;
	myTimeout=timeout;
	update();
}

AsyncRequest::~AsyncRequest()
{
	// Delete stanza..
	if (myStanza)
		delete myStanza;
	if (mySource)
		delete mySource;
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

QString AsyncRequest::id() const
{
	if (!myStanza)
		return QString::null;
	return QString::fromStdString(myStanza->findAttribute("id"));
}

