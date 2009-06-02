/***************************************************************************
 *   Copyright (C) 2009 by Dmitry Nezhevenko                               *
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
#include "muchistory.h"

#include "conference.h"
#include "nick.h"

#include "base/datastorage.h"

#include <QtDebug>
#include <QVariant>
#include <QSqlError>

MucHistory::MucHistory(int conferenceId)
{
	conferenceId_ = conferenceId;
}

MucHistory::~MucHistory()
{
}

void MucHistory::log(Nick *nick, ActionType type, const QString& msg, bool priv,
		const QString& params, const QDateTime& dateTime)
{
	QDateTime date;
	if (dateTime.isValid())
		date=dateTime;
	else
		date = QDateTime::currentDateTime();
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"INSERT INTO conference_log(conference_id, datetime, private, nick_id,"
			" action_type, message, params)"
			" VALUES(?, ?, ?, ?, ?, ?, ?)");
	q.addBindValue(nick->conference()->id());
	q.addBindValue(date);
	q.addBindValue(priv);
	q.addBindValue(nick->id());
	q.addBindValue(type);
	q.addBindValue(msg);
	q.addBindValue(params);
	if (!q.exec())
	{
		qDebug() << "SQL: Unable to log event: " << q.lastError().text();
	}
}
