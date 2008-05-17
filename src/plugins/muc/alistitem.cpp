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
#include "alistitem.h"

AListItem::AListItem(int id)
{
	id_=id;
	matcherType_=UNKNOWN;
	isRegExp_=false;
}

AListItem::AListItem(int id, MatcherType matcherType, bool isRegExp,
		const QString& value, const QDateTime& expire)
{
	id_=id;
	matcherType_=matcherType;
	isRegExp_=isRegExp;
	value_=value;
	expire_=expire;
}

AListItem::~AListItem()
{
}

bool AListItem::operator==(const AListItem& other)
{
	return matcherType_==other.matcherType() && isRegExp_ == other.isRegExp()
	&& value_==other.value() && expire_==other.expire();
}
