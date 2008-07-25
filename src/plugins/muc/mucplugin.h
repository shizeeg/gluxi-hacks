#ifndef MUCPLUGIN_H
#define MUCPLUGIN_H

#include "base/baseplugin.h"
#include "conferencelist.h"
#include "alistitem.h"

class MessageParser;
class AListItem;
class AsyncRequest;

class MucPlugin : public BasePlugin
{
	Q_OBJECT
public:
	MucPlugin(GluxiBot *parent = 0);
	~MucPlugin();
	virtual QString name() const { return "Muc"; };
	virtual QString prefix() const { return "MUC"; };
	virtual QString description() const { return "MUC support"; };
	virtual QString help() const { return ""; };
	virtual void onConnect();
	virtual void onDisconnect();
	virtual bool canHandlePresence(gloox::Stanza* s);
	virtual bool canHandleMessage(gloox::Stanza* s);
	virtual void onPresence(gloox::Stanza* );
	virtual bool parseMessage(gloox::Stanza* );
	virtual bool canHandleIq( gloox::Stanza* );
	virtual bool onIq(gloox::Stanza* );
	virtual bool onVCard(const VCardWrapper& vcard);
	virtual bool isMyMessage(gloox::Stanza*);
	virtual StorageKey getStorage(gloox::Stanza*s );
	virtual QString getJID(gloox::Stanza*s, const QString&nick);
	virtual QString getBotJID(gloox::Stanza* s);
	virtual QString JIDtoNick(const QString& jid);
	virtual QString getMyNick(gloox::Stanza* s);
	virtual QString resolveMyNick(gloox::Stanza* s);
	virtual void onQuit(const QString& reason);
	virtual AbstractConfigurator* getConfigurator(gloox::Stanza* s);
private:
	bool lazyOffline;
	ConferenceList conferences;
	QStringList confInProgress;
	QString getItem(gloox::Stanza*, const QString& name);
	void join(const QString& name, const QString& joinerBareJid=QString::null);
	void leave(const QString& name);
	Conference* getConf(gloox::Stanza* s);
	Nick* getNick(gloox::Stanza* s, const QString& nick=QString::null);
	Nick* getNickVerbose(gloox::Stanza* s, const QString& nick=QString::null);
	bool isFromConfModerator(gloox::Stanza* s);
	bool isFromConfAdmin(gloox::Stanza* s);
	bool isFromConfOwner(gloox::Stanza* s);
	void setRole(gloox::Stanza* s, Nick*, const QString& role, const QString& reason=QString::null);
	void setRole(Conference* conf, Nick*, const QString& role, const QString& reason=QString::null);
	void setAffiliation(gloox::Stanza* s, Nick*, const QString& affiliation, const QString& reason=QString::null);
	void setAffiliation(Conference* conf, const QString& jid, const QString& affiliation, const QString& reason=QString::null);
	QString getIqError(gloox::Stanza* s);
	AListItem* aFind(AList* list, Nick *n, gloox::Stanza* s, AListItem::MatcherType matcher=AListItem::MatcherUnknown);
	// Advanced commands
	bool autoLists(gloox::Stanza* s, MessageParser& parser);
	AList* alistByName(Conference* conf, const QString& name);
	int parseAListItem(gloox::Stanza* s, MessageParser& parser, AListItem& item);

	void checkMember(gloox::Stanza* s, Conference* c, Nick*, AListItem::MatcherType matcher=AListItem::MatcherUnknown);
	void recheckJIDs(Conference* c);
	void sendMessage(Conference* conf, const QString&msg);
	QRegExp getConfExp(const QString&);
	QString affiliationByCommand(const QString& cmd);
	int getStatus(gloox::Stanza* s);
	bool warnImOwner(gloox::Stanza* s);
	int getRoleForNick(Conference* conf, Nick* nick);
	QString expandMacro(gloox::Stanza* s, Conference*c, Nick* n, const QString& str, const AListItem* item=0);
	void requestVCard(gloox::Stanza* s, Conference* conf, Nick* nick);
	void requestVersion(gloox::Stanza* s, Conference* conf, Nick* nick);
private slots:
	void sltAutoLeaveTimerTimeout();
	void sltVersionQueryTimeout(AsyncRequest* req);
};

#endif
