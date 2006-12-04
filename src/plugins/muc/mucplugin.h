#ifndef MUCPLUGIN_H
#define MUCPLUGIN_H

#include "base/baseplugin.h"
#include "conferencelist.h"

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
	virtual void onQuit(const QString& reason);
private:
	ConferenceList conferences;
	QStringList confInProgress;
	QString getItem(gloox::Stanza*, const QString& name);
	QString getPresence(const gloox::Presence& pr);
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
	QString getIqError(gloox::Stanza* s);
	bool aFind(AList* list, const QString& jid, bool nickOnly=false);
	// Advanced commands
	bool autoLists(gloox::Stanza* s);
	void checkNick(Conference*c , Nick*n, const QString&, bool nickOnly=false);
	void checkJID(Conference* c, Nick*);
	void recheckJIDs(Conference* c);
	void sendMessage(Conference* conf, const QString&msg);
	QRegExp getConfExp(const QString&);
};

#endif
