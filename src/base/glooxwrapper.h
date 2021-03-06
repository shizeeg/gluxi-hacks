#ifndef GLOOXWRAPPER_H
#define GLOOXWRAPPER_CPP

#include "vcardwrapper.h"

#include <QThread>
#include <QMutex>

#include <gloox/connectiontcpclient.h>
#include <gloox/client.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagehandler.h>
#include <gloox/iqhandler.h>
#include <gloox/discohandler.h>
#include "gloox/myvcardhandler.h"
#include "gloox/myvcardmanager.h"

class MyStanza;

class GlooxWrapper: public QThread, gloox::ConnectionListener, gloox::PresenceHandler, gloox::MessageHandler, 
	public gloox::IqHandler, public gloox::DiscoHandler, public gloox::MyVCardHandler
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
	void addIqHandler(const QString& service);
	QString fetchVCard(const QString& jid);
	void setPresence(gloox::Presence presence, const QString& status, int priority);
	void setPresence(const QString& jid, gloox::Presence presence, const QString& status);
	gloox::MyVCardManager* getVCardManager() { return vcardManager; }
	std::string getID();
	gloox::JID jid();
protected:
	void run();
private:
	QMutex mutex;
	gloox::Client* myClient;
	gloox::ConnectionTCPClient* myConnection;
	gloox::MyVCardManager* vcardManager;
	
	virtual void handlePresence( gloox::Stanza *stanza );
	virtual bool handleIq(gloox::Stanza*);
	virtual bool handleIqID(gloox::Stanza*, int);
	virtual void handleDiscoInfoResult (gloox::Stanza *stanza, int context);
	virtual void handleDiscoItemsResult (gloox::Stanza *stanza, int context);
	virtual void handleDiscoError (gloox::Stanza *stanza, int context);
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e);
	virtual bool onTLSConnect( const gloox::CertInfo& );
	virtual void handleVCard (const std::string& id, const gloox::JID &jid, gloox::VCard *vcard);
	virtual void handleVCardResult (const std::string& id, MyVCardHandler::VCardContext context, 
			const gloox::JID &jid, gloox::StanzaError se=gloox::StanzaErrorUndefined);
	
signals:
	void sigMessage(const MyStanza&);
	void sigPresence(const MyStanza&);
	void sigIq(const MyStanza&);
	void sigVCard(const VCardWrapper&);
	void sigConnect();
	void sigDisconnect();
};

#endif

