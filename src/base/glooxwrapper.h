#ifndef GLOOXWRAPPER_H
#define GLOOXWRAPPER_CPP

#include <QThread>
#include <QMutex>

#include <gloox/connectiontcpclient.h>
#include <gloox/client.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagehandler.h>
#include <gloox/iqhandler.h>
#include <gloox/discohandler.h>

class gloox::Client;
class gloox::Stanza;
class MyStanza;

class GlooxWrapper: public QThread, gloox::ConnectionListener, gloox::PresenceHandler, gloox::MessageHandler, 
	public gloox::IqHandler, public gloox::DiscoHandler
{
	Q_OBJECT
public:
	GlooxWrapper();
	~GlooxWrapper();
//	gloox::Client* client() {return myClient; };
        virtual void handleMessage(gloox::Stanza*, gloox::MessageSession*);
	
	// These members should be thread-safe
	void disconnect();
	void send(gloox::Stanza* s);
	void registerIqHandler(const QString& service);
	std::string getID();
	gloox::JID jid();
protected:
	void run();
private:
	QMutex mutex;
	gloox::Client* myClient;
	gloox::ConnectionTCPClient* myConnection;
	
	virtual void handlePresence( gloox::Stanza *stanza );
	virtual bool handleIq(gloox::Stanza*);
	virtual bool handleIqID(gloox::Stanza*, int);
	virtual void handleDiscoInfoResult (gloox::Stanza *stanza, int context);
	virtual void handleDiscoItemsResult (gloox::Stanza *stanza, int context);
	virtual void handleDiscoError (gloox::Stanza *stanza, int context);
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e);
	virtual bool onTLSConnect( const gloox::CertInfo& );

signals:
	void sigMessage(const MyStanza&);
	void sigPresence(const MyStanza&);
	void sigIq(const MyStanza&);
	void sigConnect();
};

#endif

