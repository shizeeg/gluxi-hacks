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
#include "base/common.h"

AListItem::AListItem(int id)
{
	id_=id;
	matcherType_=MatcherUnknown;
	testType_=TestExact;
	invert_=false;
	child_=0;
}

AListItem::AListItem(int id, MatcherType matcherType, TestType testType,
		const QString& value, const QDateTime& expire)
{
	id_=id;
	matcherType_=matcherType;
	testType_=testType;
	value_=value;
	expire_=expire;
	invert_=false;
	child_=0;
}

AListItem::~AListItem()
{
	if (child_)
		delete child_;
}

bool AListItem::operator==(const AListItem& other)
{
	return matcherType_==other.matcherType() && testType_ == other.testType()
		&& value_==other.value() && expire_==other.expire();
}

bool AListItem::isSameCondition(const AListItem& other)
{
	if (other.child() && !child_)
		return false;
	if (child_ && !other.child())
		return false;
	if (child_ && !child_->isSameCondition(*other.child()))
		return false;
	return matcherType_==other.matcherType() && testType_ == other.testType()
			&& value_==other.value();
}

QString AListItem::toString() const
{
	QString flags;
	flags+=isInvert() ? "!" : " ";

	switch (matcherType())
	{
	case AListItem::MatcherUnknown:
		flags+="? ";
		break;
	case AListItem::MatcherNick:
		flags+="N ";
		break;
	case AListItem::MatcherJid:
		flags+="J ";
		break;
	case AListItem::MatcherBody:
		flags+="B ";
		break;
	case AListItem::MatcherResource:
		flags+="R ";
		break;
	case AListItem::MatcherVersion:
		flags+="V ";
		break;
	case AListItem::MatcherVersionName:
		flags+="Vn";
		break;
	case AListItem::MatcherVersionClient:
		flags+="Vn";
		break;
	case AListItem::MatcherVersionOs:
		flags+="Vo";
		break;
	case AListItem::MatcherVCardPhotoSize:
		flags+="Ps";
	};

	switch (testType())
	{
		case AListItem::TestUnknown:
			flags+="?";
			break;
		case AListItem::TestExact:
			flags+=" ";
			break;
		case AListItem::TestRegExp:
			flags+="E";
			break;
		case AListItem::TestSubstring:
			flags+="S";
			break;
		case AListItem::TestGreater:
			flags+=">";
			break;
		case AListItem::TestLesser:
			flags+="<";
			break;
	}

	QString line=QString("%1 %2").arg(flags).arg(value());
	if (expire().isValid())
	{
		int delta=QDateTime::currentDateTime().secsTo(expire());
		if (delta>0)
			line+=QString("	[%1]").arg(secsToString(delta));
		else
			line+=QString(" [EXPIRED]");
	}
	if (!reason().isEmpty())
		line+=" // "+reason();
	if (child_)
		line+="\n   && "+child_->toString();
	return line;
}
