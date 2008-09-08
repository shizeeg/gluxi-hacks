/***************************************************************************
 *   Copyright (C) 2008 by Dmitry Nezhevenko                               *
 *   dion@inhex.net                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "rosterplugin.h"
#include "rosterstorage.h"

#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/datastorage.h"


#include <QtDebug>
#include <QTime>

RosterPlugin::RosterPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	// Storage provider
	pluginId=2;
	// Let muc plugin provide storage for conference stuff
	priority_=100;
	rosterStorage_=new RosterStorage();
	autoCreateStorage_=(DataStorage::instance()->getString("roster/autocreate_storage")=="1");
}

RosterPlugin::~RosterPlugin()
{
	delete rosterStorage_;
	rosterStorage_=0l;
}

StorageKey RosterPlugin::getStorage(gloox::Stanza*s)
{
	if (!rosterStorage_)
		return StorageKey();
	QString jid=QString::fromStdString(s->from().bare());
	int idx=rosterStorage_->getStorage(jid);
	if (idx<=0 && autoCreateStorage_)
		idx=rosterStorage_->createStorage(jid);
	if (idx<=0)
		return StorageKey();
	return StorageKey(pluginId, idx);
}
