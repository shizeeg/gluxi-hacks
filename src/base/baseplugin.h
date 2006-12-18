#ifndef BASEPLUGIN_H
#define BASEPLUGIN_H

#include <QObject>
#include <QStringList>

#include <gloox/client.h>

class GluxiBot;

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class BasePlugin : public QObject
{
	Q_OBJECT
public:
	BasePlugin(GluxiBot *parent);
	~BasePlugin();
	int id() const {return pluginId;};
	virtual QString name() const { return "BasePlugin"; };
	virtual QString prefix() const { return "base"; };
	virtual QString help() const { return QString::null; };
	virtual QString description() const { return "base plugin"; };
	virtual void onConnect();
	virtual bool allMessages() const { return false; };
	virtual bool onMessage(gloox::Stanza* );
	virtual bool parseMessage(gloox::Stanza*);
 	virtual void onPresence(gloox::Stanza* );
 	virtual bool canHandleMessage(gloox::Stanza* );
 	virtual bool canHandlePresence(gloox::Stanza* );
 	virtual bool canHandleIq(gloox::Stanza*);
 	virtual bool onIq(gloox::Stanza*);
 	virtual bool shouldIgnoreError(); // Should we ignore can't handle error;
 	virtual bool isMyMessage(gloox::Stanza*s); // TODO: Fix it
 	virtual int getStorage(gloox::Stanza*s);
	virtual QString getJID(gloox::Stanza*s, const QString&nick);
	virtual QString JIDtoNick(const QString& jid);
	virtual void onQuit(const QString& reason);			// OnQuit
	void reply(gloox::Stanza*, const QString&, bool forcePrivate=false);
protected:
	int pluginId;
	bool myShouldIgnoreError;
	QStringList commands;
	QString lprefix() const { return prefix().toLower(); };
	GluxiBot* bot();
	bool isGroupChat(gloox::Stanza* );
 	bool isOfflineMessage(gloox::Stanza*);
	QString getBody(gloox::Stanza*, bool usePrefix=true );
	QString getNick(gloox::Stanza*);

	virtual bool isFromBotOwner(gloox::Stanza*, bool message=false);
	int getRole(gloox::Stanza*);
};

#endif

