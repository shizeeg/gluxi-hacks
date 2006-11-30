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
	AsyncRequest(int id, int timeout=600);
	AsyncRequest(int id, BasePlugin *plugin, const QString& name, gloox::Stanza *stanza=0, int timeout=600);
	~AsyncRequest();
	int id() const { return myId; };
	BasePlugin* plugin() const { return myPlugin; };
	QString name() const { return myName; };
	gloox::Stanza *stanza() const { return myStanza; };
	gloox::Stanza *source() const { return mySource; };
	int timeout() const { return myTimeout; };
	QDateTime time() const { return myTime; };

	void setId(int i) { myId=i; };
	void setPlugin(BasePlugin *p) { myPlugin=p; };
	void setName(const QString& n) { myName=n; };
	void setStanza(gloox::Stanza *s) { myStanza=s; };
	void setSource(gloox::Stanza *s) { mySource=s; };
	void setTimeout(int t) { myTimeout=t; };
	void update();
	bool expired();
	QString stanzaId() const;
private:
	int myId;
	BasePlugin* myPlugin;
	QString myName;
	gloox::Stanza *myStanza;
	gloox::Stanza *mySource;
	int myTimeout;
	QDateTime myTime;
	bool notified;
signals:
	void onExpire();
};

#endif

