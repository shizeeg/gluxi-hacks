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
#include "identityitem.h"

IdentityItem::IdentityItem(const QString& category, const QString& type, const QString& name)
{
	category_=category;
	type_=type;
	name_=name;
}

IdentityItem::~IdentityItem()
{
}

gloox::Tag* IdentityItem::infoTag()
{
	gloox::Tag* tag=new gloox::Tag("identity");
	tag->addAttribute("category", category_.toStdString());
	tag->addAttribute("type",type_.toStdString());
	tag->addAttribute("name", name_.toStdString());
	return tag;
}
