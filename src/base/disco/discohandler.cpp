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
#include "discohandler.h"
#include "featureitem.h"

#include <gloox/stanza.h>

DiscoHandler::DiscoHandler(const QString& node, const QString& parentNode, const QString& name, const QString& jid)
{
	node_=node;
	parentNode_=parentNode;
	name_=name;
	jid_=jid;
	addInfoItem(new FeatureItem("http://jabber.org/protocol/disco#info"));
	addInfoItem(new FeatureItem("http://jabber.org/protocol/disco#items"));
}

DiscoHandler::~DiscoHandler()
{
}

void DiscoHandler::addInfoItem(InfoItem* item)
{
	infoItems_.append(item);
}

void DiscoHandler::addChildHandler(DiscoHandler* handler)
{
	childDiscoHandlers_.append(handler);
}

gloox::Stanza* DiscoHandler::handleDiscoRequest(gloox::Stanza* s)
{
	QString xmlns=QString::fromStdString(s->xmlns());
	if (s->subtype()!=gloox::StanzaIqGet)
		return 0;
	if (xmlns=="http://jabber.org/protocol/disco#info")
		return handleDiscoInfoRequest(s);
	if (xmlns=="http://jabber.org/protocol/disco#items")
			return handleDiscoItemsRequest(s);
	return 0;
}

gloox::Stanza* DiscoHandler::handleDiscoInfoRequest(gloox::Stanza* s)
{
	gloox::Tag* queryTag=s->findChild("query");
	if (queryTag==0)
		return false;
	if (queryTag->findAttribute("node")!=node_.toStdString())
		return false;
	gloox::Stanza* out=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult,"http://jabber.org/protocol/disco#info");
	gloox::Tag* queryOut=out->findChild("query");
	for (QList<InfoItem*>::iterator it=infoItems_.begin(); it!=infoItems_.end(); ++it)
	{
		gloox::Tag* sub=(*it)->infoTag();
		if (sub)
			queryOut->addChild(sub);
	}
	return out;
}

gloox::Stanza* DiscoHandler::handleDiscoItemsRequest(gloox::Stanza* s)
{
	gloox::Tag* queryTag=s->findChild("query");
	if (queryTag==0)
		return false;
	if (queryTag->findAttribute("node")!=node_.toStdString())
		return false;
	gloox::Stanza* out=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult,"http://jabber.org/protocol/disco#items");
	gloox::Tag* queryOut=out->findChild("query");
	QString toJid=QString::fromStdString(s->to().full());
	for (QList<DiscoHandler*>::iterator it=childDiscoHandlers_.begin(); it!=childDiscoHandlers_.end(); ++it)
	{
		gloox::Tag* sub=(*it)->itemTag(toJid);
		if (sub)
			queryOut->addChild(sub);
	}
	return out;
}

gloox::Tag* DiscoHandler::itemTag(const QString& defaultJid)
{
	gloox::Tag* tag=new gloox::Tag("item");
	tag->addAttribute("node",node_.toStdString());
	tag->addAttribute("name",name_.toStdString());
	tag->addAttribute("jid",(jid_.isEmpty() ? defaultJid : jid_).toStdString());
	return tag;
}
