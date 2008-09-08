#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include "vcardwrapper.h"
#include "config/storagekey.h"

#include <QObject>
#include <QStringList>

#include <gloox/client.h>

class GluxiBot;
class AbstractConfigurator;

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class BasePlugin : public QObject
{
	Q_OBJECT
public:
	BasePlugin(GluxiBot *parent);
	virtual ~BasePlugin();
	int id() const {return pluginId;};
	int priority() const { return priority_; };
	virtual QString name() const { return "BasePlugin"; };
	virtual QString prefix() const { return "base"; };
	virtual QString help() const { return QString::null; };
	virtual QString description() const { return "base plugin"; };
	virtual void onConnect();
	virtual void onDisconnect();
	virtual bool allMessages() const { return false; };
	virtual bool onMessage(gloox::Stanza* );
	virtual bool onVCard(const VCardWrapper& vcard);
	virtual bool parseMessage(gloox::Stanza*);
 	virtual void onPresence(gloox::Stanza* );
 	virtual bool canHandleMessage(gloox::Stanza* );
 	virtual bool canHandlePresence(gloox::Stanza* );
 	virtual bool canHandleIq(gloox::Stanza*);
 	virtual bool onIq(gloox::Stanza*);
 	virtual bool shouldIgnoreError(); // Should we ignore can't handle error;
 	virtual bool isMyMessage(gloox::Stanza*s); // TODO: Fix it
 	virtual StorageKey getStorage(gloox::Stanza*s);
	virtual QString getJID(gloox::Stanza*s, const QString&nick, bool realJid=false);
	virtual QString getBotJID(gloox::Stanza* s);
	virtual QString JIDtoNick(const QString& jid);
	virtual void onQuit(const QString& reason);			// OnQuit
	void reply(gloox::Stanza*, const QString&, bool forcePrivate=false, bool quoteNick=true);
	virtual QString getMyNick(gloox::Stanza* s);
	virtual QString resolveMyNick(gloox::Stanza* s);
	virtual AbstractConfigurator* getConfigurator(gloox::Stanza* s);
protected:
	int pluginId;
	int priority_;
	bool myShouldIgnoreError;
	QStringList commands;
	QString lprefix() const { return prefix().toLower(); };
	GluxiBot* bot();
	bool isGroupChat(gloox::Stanza* );
 	bool isOfflineMessage(gloox::Stanza*);
	QString getNick(gloox::Stanza*);

	virtual bool isFromBotOwner(gloox::Stanza*, bool message=false);
	int getRole(gloox::Stanza*);
	int getRole(const QString& jid);
	QString getPresence(const gloox::Presence& pr);
};

#endif

