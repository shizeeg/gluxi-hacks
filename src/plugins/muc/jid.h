#ifndef JID_H
#define JID_H

#include <QString>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/

class Nick;
class Conference;

class Jid{
public:
    Jid(Nick *parent, const QString& fullJid=QString::null);
    ~Jid();
	int id() const { return myId; };
    QString jid() const { return myJid; };
    void setFullJid(const QString& );
    void commit();
    void remove();
    static void removeTemporary(Conference *conf=0);
private:
	Nick* myParent;
	bool myTemporary;
	int myId;
	QString myJid;
	QString myResource;
	void loadJid();
};

#endif
