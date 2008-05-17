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
#ifndef ALISTITEM_H_
#define ALISTITEM_H_

#include <QString>
#include <QDateTime>

class AListItem
{
public:
	enum MatcherType
	{
		UNKNOWN=0,
		NICK=1,
		JID=2,
		BODY=3
	};
	AListItem(int id=-1);
	AListItem(int id, MatcherType matcherType, bool isRegExp=false, const QString& value=QString::null, const QDateTime& expire=QDateTime());
	virtual ~AListItem();
	
	int id() const { return id_; }
	MatcherType matcherType() const { return matcherType_; }
	bool isRegExp() const { return isRegExp_; }
	QString value() const { return value_; }
	QString reason() const { return reason_; }
	QDateTime expire() const { return expire_; }
	
	void setId(int id) { id_=id; }
	void setMatcherType(MatcherType matcherType) { matcherType_=matcherType; }
	void setIsRegExp(bool isRegExp) { isRegExp_=isRegExp; }
	void setValue(const QString& value) { value_=value; }
	void setReason(const QString& reason) { reason_=reason; }
	void setExpire(const QDateTime& expire) { expire_=expire; }
	
	bool operator==(const AListItem& other);
private:
	int id_;
	MatcherType matcherType_;
	bool isRegExp_;
	QString value_;
	QString reason_;
	QDateTime expire_;
};

#endif /*ALISTITEM_H_*/
