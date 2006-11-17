#ifndef NICK_H
#define NICK_H

#include <QString>
#include <QDateTime>

class Conference;
class Jid;

class Nick{
public:
    Nick(Conference* parent, const QString& nick, const QString& jid=QString::null);
    ~Nick();

	QString jid() const { return myJidS; };
	QString nick() const { return myNick; };
	QString affiliation() const { return myAffiliation; };
	QString role() const { return myRole; };
	QDateTime joined() const { return myJoined; };
	QDateTime lastActivity() const { return myLastActivity; };
	QString show() const { return myShow; };
	QString status() const { return myStatus; };
	Conference* conference() const { return myParent; };

	void setJid(const QString& jid);
	void setNick(const QString& nick);
	void setAffiliation(const QString& affiliation) { myAffiliation=affiliation; };
	void setRole(const QString& role) { myRole=role; };
	void updateLastActivity();
	void setShow(const QString& show) { myShow=show; };
	void setStatus(const QString& status) { myStatus=status; };
	void commit();
	static void setAllOffline (Conference* conf);
private:
	int myId;
	Conference *myParent;
	Jid *myJid;
	QString myJidS;
	QString myNick;
	QString myAffiliation;
	QString myRole;
	QDateTime myJoined;
	QDateTime myLastActivity;
	QString myShow;
	QString myStatus;
};

#endif
