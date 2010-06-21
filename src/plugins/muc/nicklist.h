#ifndef NICKLIST_H
#define NICKLIST_H

#include "nick.h"
#include "jid.h"
#include <QList>
/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class Jid;

class NickList: public QList<Nick*>
{
public:
	NickList();
	~NickList();
	void clear();
	void justClear();
	void lazyClear();
	void remove(Nick* nick);
	Nick *byName(const QString&) const;
	Nick *byJid(const QString&) const;
};

#endif
