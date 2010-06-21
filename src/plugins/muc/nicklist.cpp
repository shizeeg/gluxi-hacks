//
// C++ Implementation: nicklist
//
// Description:
//
//
// Author: Dmitry Nezhevenko <dion@inhex.net>, (C) 2006
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "nicklist.h"
#include <QtDebug>

NickList::NickList()
{
}

NickList::~NickList()
{
	clear();
}

void NickList::clear()
{
	while (count())
		delete takeAt(0);
}

void NickList::justClear()
{
	QList<Nick*>::clear();
}

void NickList::lazyClear()
{
	while (count())
	{
		Nick* nick=takeAt(0);
		nick->setLazyLeave(true);
		delete nick;
	}
}

Nick* NickList::byName(const QString& name) const
{
	int cnt=count();
	Nick *nick;
	for (int i=0; i<cnt; i++)
	{
		nick=value(i);
		if (nick->nick().toUpper()==name.toUpper())
			return nick;
	}
	return 0;
}

Nick* NickList::byJid(const QString& j) const
{
	int cnt=count();
	Jid* jid;
	for (int i = 0; i < cnt; i++)
	{
	  jid = value(i)->jid();
	  if (j.toLower() == jid->jid().toLower())
		return value(i);
	}
	return 0;
}

void NickList::remove(Nick* nick)
{
	qDebug() << "[NICKLIST] Remove";
	removeAll(nick);
	delete nick;
}
