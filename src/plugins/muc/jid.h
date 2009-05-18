#ifndef JID_H
#define JID_H

#include <QString>
#include <QDateTime>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/

class Nick;
class Conference;

class Jid{
public:
	Jid(Nick *parent, const QString& fullJid=QString::null);
	Jid(Nick *parent, int id);
    	~Jid();
	int id() const { return myId; };
	QString jid() const { return myJid; };
	void setFullJid(const QString& );
	QDateTime created() const { return myCreated; };
	void commit();
	void remove();
	bool isTemporary() const { return myTemporary; }
	static void removeTemporary(Conference *conf=0);
private:
	Nick* myParent;
	bool myTemporary;
	int myId;
	QString myJid;
	QString myResource;
	QDateTime myCreated;
	void loadJid();
	void updateJid();
};

#endif
