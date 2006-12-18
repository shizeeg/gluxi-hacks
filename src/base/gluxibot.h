#ifndef GLOOXBOT_H
#define GLOOXBOT_H

#include "pluginlist.h"

#include <QObject>
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

class gloox::Client;
class gloox::Stanza;

class GluxiBot: public QObject
{
	Q_OBJECT
public:
	GluxiBot();
	~GluxiBot();
	gloox::Client* client();
	GlooxWrapper *gloox() { return myGloox; };

	RoleList *roles() { return myRoles; };
	// Depreacted members
//	QStringList* owners() { return &myOwners; };
//	QStringList* tmpOwners() { return &myTmpOwners; };
	
	PluginList* plugins() { return &myPlugins; };
	AsyncRequestList* asyncRequests() { return myAsyncRequests; };
	QList<int> getStorage(gloox::Stanza*s);

	bool isMyMessage(gloox::Stanza *);
	QString getJID(gloox::Stanza*s, const QString&);
	QString JIDtoNick(const QString& jid);

	void onQuit(const QString&reason);
private:
	GlooxWrapper *myGloox;

	RoleList *myRoles;

	// Deprecated
	// Bot owners (Real jids)
//	QStringList myOwners;
	// Bot temporary owners: owners conference jids
//	QStringList myTmpOwners;
	
	PluginList myPlugins;
	AsyncRequestList *myAsyncRequests;
	BasePlugin* pluginByStanzaId(gloox::Stanza*);

private slots:
	void handleMessage(const MyStanza&);
	void handlePresence(const MyStanza&);
	void handleIq(const MyStanza&);
	void onConnect();

private slots:
};

#endif

