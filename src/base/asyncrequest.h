#ifndef ASYNCREQUEST_H
#define ASYNCREQUEST_H

#include <QObject>
#include <QDateTime>

#include <gloox/stanza.h>

//NOTE: Stanza will be removed here in destructor
class BasePlugin;

class AsyncRequest: public QObject
{
	Q_OBJECT
public:
	AsyncRequest(int timeout=600);
	AsyncRequest(BasePlugin *plugin, gloox::Stanza *stanza, int timeout=600);
	~AsyncRequest();
	BasePlugin* plugin() const { return myPlugin; };
	gloox::Stanza *stanza() const { return myStanza; };
	gloox::Stanza *source() const { return mySource; };
	int timeout() const { return myTimeout; };

	void setPlugin(BasePlugin *p) { myPlugin=p; };
	void setStanza(gloox::Stanza *s) { myStanza=s; };
	void setSource(gloox::Stanza *s) { mySource=s; };
	void setTimeout(int t) { myTimeout=t; };
	void update();
	bool expired();
	QString id() const;
private:
	BasePlugin* myPlugin;
	gloox::Stanza *myStanza;
	gloox::Stanza *mySource;
	int myTimeout;
	QDateTime myTime;
	bool notified;
signals:
	void onExpire();
};

#endif

