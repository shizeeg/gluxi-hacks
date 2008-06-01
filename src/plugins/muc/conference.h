#ifndef CONFERENCE_H
#define CONFERENCE_H

#include "nicklist.h"

#include <QString>
#include <QStringList>

class AList;
class MucConfigurator;

class Conference
{
public:
	Conference();
	Conference(const QString& name, const QString& nick, bool lazyLeave=false);
	~Conference();
	int id() const { return myId;};
	QString nick() const { return myNick;};
	QString name() const { return myName; };
	NickList* nicks() { return &myNicks; };
	bool lazyLeave() const { return myLazyLeave; }
	bool validated() const { return myValidated; }

	static QStringList autoJoinList(); // List conferences to autojoin
	void setAutoJoin(bool b);
	void removeExpired();

	AList* aban() { return myBan; };
	AList* akick() { return myKick; };
	AList* avisitor() { return myVisitor; };
	AList* amoderator() { return myModerator; };
	AList* acommand() { return myCommand; };
	QString seen(const QString& nick);
	QString clientStat();
	void setNick(const QString& name);
	void setLazyLeave(bool value);
	void setValidated(bool value) { myValidated=value; }
	
	void loadOnlineNicks();
	void cleanNonValidNicks();

	MucConfigurator* configurator() const { return configurator_; }
	void setConfigurator(MucConfigurator* configurator) { configurator_=configurator; }
private:
	int myId;
	bool myLazyLeave;
	bool myValidated;
	QString myNick;
	QString myName;
	NickList myNicks;
	/*QStringList myKick;
	QStringList myVisitor;
	QStringList myModerator;*/
	AList* myBan;
	AList* myKick;
	AList* myVisitor;
	AList* myModerator;
	AList* myCommand;
	MucConfigurator* configurator_;
};

#endif
