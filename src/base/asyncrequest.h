#ifndef ASYNCREQUEST_H
#define ASYNCREQUEST_H

#include <QObject>
#include <QDateTime>

namespace gloox
{
	class Stanza;
}

class BasePlugin;

class AsyncRequest: public QObject
{
	Q_OBJECT
public:
	AsyncRequest(int id, BasePlugin *plugin, gloox::Stanza *from, int timeout=600);
	~AsyncRequest();
	int id() const { return myId; };
	BasePlugin* plugin() const { return myPlugin; };
	QString name() const { return myName; };
	gloox::Stanza *stanza() const { return myStanza; };
	int timeout() const { return myTimeout; };
	QDateTime time() const { return myTime; };
	void setId(int i) { myId=i; };
	void setPlugin(BasePlugin *p) { myPlugin=p; };
	void setName(const QString& n) { myName=n; };
	void setStanza(gloox::Stanza *s) { myStanza=s; };
	void setTimeout(int t) { myTimeout=t; };
	void update();
	bool expired();
	QString stanzaId() const;
private:
	int myId;
	BasePlugin* myPlugin;
	QString myName;
	gloox::Stanza* myStanza;
	int myTimeout;
	QDateTime myTime;
	bool notified;
signals:
	void wantDelete(AsyncRequest*);
	void onExpire();
};

#endif

