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
#ifndef ACLPLUGIN_H
#define ACLPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

#include <QMap>

class AclList;
class MessageParser;

class AclPlugin : public BasePlugin
{
	Q_OBJECT
public:
	AclPlugin(GluxiBot *parent = 0);
	~AclPlugin();
	virtual QString name() const { return "ACL"; };
	virtual QString prefix() const { return "ACL"; };
	virtual bool canHandleMessage(gloox::Stanza* s);
	virtual bool parseMessage(gloox::Stanza*);
	bool parseCommands(gloox::Stanza* s, MessageParser& parser);
private:
	AclList* aclList_;
	QMap<QString, QString> aclMap_;
	bool isJidAccepted(const QString& jid);
	int getAccessLevel(const QString& key);
};

#endif
