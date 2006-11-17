#ifndef GLOOXBOT_H
#define GLOOXBOT_H

#include "pluginlist.h"

#include <QStringList>

#include <gloox/client.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagehandler.h>
#include <gloox/iqhandler.h>

class gloox::Client;
class gloox::Stanza;

class GluxiBot: public QObject, gloox::ConnectionListener, gloox::PresenceHandler, gloox::MessageHandler, gloox::IqHandler
{
	Q_OBJECT
public:
	GluxiBot();
	~GluxiBot();
	void run();
	gloox::Client* client() {return myClient; };
	QStringList* owners() { return &myOwners; };
	QStringList* tmpOwners() { return &myTmpOwners; };
	PluginList* plugins() { return &myPlugins; };
	QList<int> getStorage(gloox::Stanza*s);

	bool isMyMessage(gloox::Stanza *);
	virtual void handleMessage(gloox::Stanza*);
	QString getJID(gloox::Stanza*s, const QString&);
	QString JIDtoNick(const QString& jid);
private:
	gloox::Client* myClient;
	QStringList myOwners;
	QStringList myTmpOwners;
	PluginList myPlugins;

	virtual void handlePresence( gloox::Stanza *stanza );
	virtual bool handleIq(gloox::Stanza*);
	virtual bool handleIqID(gloox::Stanza*, int);
	virtual void onConnect();
	virtual void onDisconnect( gloox::ConnectionError e);
	virtual bool onTLSConnect( const gloox::CertInfo& );
};

#endif

