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
#ifndef DISCOHANDLER_H_
#define DISCOHANDLER_H_

#include "infoitem.h"

#include <gloox/stanza.h>

#include <QList>
#include <QString>

class GluxiBot;

class gloox::Stanza;

class DiscoHandler
{
public:
	DiscoHandler(const QString& node=QString(), const QString& parentNode=QString(), const QString& name=QString());
	virtual ~DiscoHandler();
	void addInfoItem(InfoItem* item);
	void addChildHandler(DiscoHandler* handler);
	void removeChildHandler(DiscoHandler* handler);
	
	virtual gloox::Stanza* handleDiscoRequest(gloox::Stanza* s, const QString& jid);
	virtual gloox::Tag* itemTag(const QString& jid);
	QString node() const { return node_; }
	QString parentNode() const { return parentNode_; }
	QString name() const { return name_; }
	void setBot(GluxiBot* bot) { bot_=bot; }
protected:
	QString node_;
	QString parentNode_;
	QString name_;
	QList<InfoItem*> infoItems_;
	QList<DiscoHandler*> childDiscoHandlers_;
	GluxiBot* bot_;
	gloox::Stanza* handleDiscoInfoRequest(gloox::Stanza* s, const QString& jid);
	gloox::Stanza* handleDiscoItemsRequest(gloox::Stanza* s, const QString& jid);
};

#endif /*DISCOHANDLER_H_*/
