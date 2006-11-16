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

Nick* NickList::byName(const QString& name)
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

void NickList::remove(Nick* nick)
{
	qDebug() << "[NICKLIST] Remove";
	removeAll(nick);
	delete nick;
}
