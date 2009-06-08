#ifndef CONFERENCE_H
#define CONFERENCE_H

#include "nicklist.h"

#include <QString>
#include <QStringList>

class AList;
class MucConfigurator;
class MucHistory;

class Conference
{
public:
	Conference();
	Conference(const QString& name, const QString& nick, bool lazyLeave=false);
	~Conference();
	int id() const { return myId;};
	QString nick() const { return myNick;};

	Nick *botNick() const;

	QString name() const { return myName; };
	NickList* nicks() { return &myNicks; };
	bool lazyLeave() const { return myLazyLeave; }
	bool validated() const { return myValidated; }

	static QStringList autoJoinList(); // List conferences to autojoin
	static QStringList autoLeaveList(); // List 'died' conferences to leave
	static void disableAutoJoin(const QString& conference);

	void setAutoJoin(bool b);
	void removeExpired();

	AList* aban() { return myBan; };
	AList* akick() { return myKick; };
	AList* avisitor() { return myVisitor; };
	AList* amoderator() { return myModerator; };
	AList* aparticipant() { return myParticipant; }
	AList* acommand() { return myCommand; };
	QStringList* alistTraceList() { return alistTraceList_; }

	QString seen(const QString& nick, bool ext = false);
	QString clientStat();
	void setNick(const QString& name);
	void setLazyLeave(bool value);
	void setValidated(bool value) { myValidated=value; }

	void loadOnlineNicks();
	void cleanNonValidNicks();
	void markOffline();

	MucConfigurator* configurator() const { return configurator_; }
	void setConfigurator(MucConfigurator* configurator) { configurator_=configurator; }

	MucHistory *history() const { return history_; }
private:
	int myId;
	bool myLazyLeave;
	bool myValidated;
	QString myNick;
	QString myName;
	NickList myNicks;
	AList* myBan;
	AList* myKick;
	AList* myVisitor;
	AList* myModerator;
	AList* myCommand;
	AList* myParticipant;
	MucConfigurator* configurator_;
	QStringList* alistTraceList_;
	MucHistory *history_;
};

#endif
