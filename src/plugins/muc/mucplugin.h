#ifndef MUCPLUGIN_H
#define MUCPLUGIN_H

#include "base/baseplugin.h"
#include "conferencelist.h"

class MessageParser;

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
	virtual bool isMyMessage(gloox::Stanza*);
	virtual int getStorage(gloox::Stanza*s );
	virtual QString getJID(gloox::Stanza*s, const QString&nick);
	virtual QString JIDtoNick(const QString& jid);
	virtual QString getMyNick(gloox::Stanza* s);
	virtual QString resolveMyNick(gloox::Stanza* s); 
	virtual void onQuit(const QString& reason);
private:
	bool lazyOffline;
	ConferenceList conferences;
	QStringList confInProgress;
	QString getItem(gloox::Stanza*, const QString& name);
	void join(const QString& name);
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
	bool aFind(AList* list, Nick *n);
	// Advanced commands
	bool autoLists(gloox::Stanza* s, MessageParser& parser);
	void checkMember(gloox::Stanza* s, Conference* c, Nick*);
	void recheckJIDs(Conference* c);
	void sendMessage(Conference* conf, const QString&msg);
	QRegExp getConfExp(const QString&);
	QString affiliationByCommand(const QString& cmd);
	int getStatus(gloox::Stanza* s);
	bool warnImOwner(gloox::Stanza* s);
};

#endif
