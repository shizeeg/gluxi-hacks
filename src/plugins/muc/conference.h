#ifndef CONFERENCE_H
#define CONFERENCE_H

#include "nicklist.h"

#include <QString>
#include <QStringList>

class AList;

class Conference
{
public:
	Conference();
	Conference(const QString& name, const QString& nick);
	~Conference();
	int id() const { return myId;};
	QString nick() const { return myNick;};
	QString name() const { return myName; };
	NickList* nicks() { return &myNicks; };

	static QStringList autoJoinList(); // List conferences to autojoin
	void setAutoJoin(bool b);
	void removeExpired();

	AList* akick() { return myKick; };
	AList* avisitor() { return myVisitor; };
	AList* amoderator() { return myModerator; };
private:
	int myId;
	QString myNick;
	QString myName;
	NickList myNicks;
	/*QStringList myKick;
	QStringList myVisitor;
	QStringList myModerator;*/
	AList* myKick;
	AList* myVisitor;
	AList* myModerator;
};

#endif
