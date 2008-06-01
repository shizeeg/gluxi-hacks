#ifndef GLOOXBOT_H
#define GLOOXBOT_H

#include "pluginlist.h"
#include "config/storagekey.h"

#include <QObject>
#include <QEvent>
#include <QStringList>

#include <gloox/client.h>
#include <gloox/presencehandler.h>
#include <gloox/connectionlistener.h>
#include <gloox/messagehandler.h>
#include <gloox/iqhandler.h>

class GlooxWrapper;
class AsyncRequestList;
class MyStanza;
class RoleList;
class VCardWrapper;
class AbstractConfigurator;
class RootDiscoHandler;

class gloox::Client;
class gloox::Stanza;

class QuitEvent: public QEvent
{
public:
        QuitEvent(const QString& msg) : QEvent(QEvent::User) , message(msg) {; }
        const QString& msg() const { return message; }
private:
        QString message;
};


class GluxiBot: public QObject
{
	Q_OBJECT
public:
	GluxiBot();
	~GluxiBot();
//	gloox::Client* client();
	GlooxWrapper *client() { return myGloox; };

	RoleList *roles() { return myRoles; };
	// Depreacted members
//	QStringList* owners() { return &myOwners; };
//	QStringList* tmpOwners() { return &myTmpOwners; };
	
	PluginList* plugins() { return &myPlugins; };
	AsyncRequestList* asyncRequests() { return myAsyncRequests; };
	StorageKey getStorage(gloox::Stanza*s);
	AbstractConfigurator* getConfigurator(gloox::Stanza* s);
	bool isMyMessage(gloox::Stanza *);
	QString getJID(gloox::Stanza*s, const QString&);
	QString getBotJID(gloox::Stanza*s);
	QString JIDtoNick(const QString& jid);
	void onQuit(const QString& reason);
	int getPriority();
	QString getMyNick(gloox::Stanza* s);
	void registerIqHandler(const QString& service);
	RootDiscoHandler* getRootDiscoHandler() { return rootDiscoHandler_; }
private:
	GlooxWrapper *myGloox;
	RootDiscoHandler* rootDiscoHandler_;

	RoleList *myRoles;
	PluginList myPlugins;
	AsyncRequestList *myAsyncRequests;
	BasePlugin* pluginByStanzaId(gloox::Stanza*);
protected:
        void customEvent(QEvent *event);
private slots:
	void handleMessage(const MyStanza&);
	void handlePresence(const MyStanza&);
	void handleIq(const MyStanza&);
	void handleVCard(const VCardWrapper& vcard);
	void onConnect();
	void onDisconnect();
};

#endif

