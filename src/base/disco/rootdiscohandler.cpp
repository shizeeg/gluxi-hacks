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
#include "rootdiscohandler.h"
#include "identityitem.h"
#include "../gluxibot.h"

#include <QDebug>

RootDiscoHandler::RootDiscoHandler(GluxiBot* bot)
{
	bot_=bot;
	rootHandler_=new DiscoHandler("");
	rootHandler_->addInfoItem(new IdentityItem("client","bot","GluxiBot"));
}

RootDiscoHandler::~RootDiscoHandler()
{
	delete rootHandler_;
}

gloox::Stanza* RootDiscoHandler::handleDiscoRequest(gloox::Stanza* s)
{	
	const QString& jid=bot_->getBotJID(s);
	gloox::Tag* queryTag=s->findChild("query");
	if (!queryTag)
		return false;
	QString node=QString::fromStdString(queryTag->findAttribute("node"));
	
	if (node.isEmpty())
		return rootHandler_->handleDiscoRequest(s, jid);
	
	DiscoHandler* handler=handlersMap_.value(node);
	if (handler)
		return handler->handleDiscoRequest(s, jid);
	return 0;
}

void RootDiscoHandler::addIqHandler(const QString& service)
{
	//TODO: Should we do anything here?
}

void RootDiscoHandler::registerDiscoHandler(DiscoHandler* handler)
{
	if (handler->parentNode().isEmpty())
	{
		rootHandler_->addChildHandler(handler);
	}
	else
	{
		DiscoHandler* parentHandler=handlersMap_.value(handler->parentNode());
		if (parentHandler)
			parentHandler->addChildHandler(handler);
	}
	
	handlersMap_.insert(handler->node(), handler);
}

void RootDiscoHandler::unregisterDiscoHandler(DiscoHandler* handler)
{
	if (handler->parentNode().isEmpty())
	{
		rootHandler_->removeChildHandler(handler);
	}
	else
	{
		DiscoHandler* parentHandler=handlersMap_.value(handler->parentNode());
		if (parentHandler)
			parentHandler->removeChildHandler(handler);
	}
	handlersMap_.remove(handler->node());
}
