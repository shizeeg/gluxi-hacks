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

void MucHistory::log(Nick *nick, Nick *dstNick, ActionType type, const QString& msg, bool priv,
		const QString& params, const QDateTime& dateTime)
{
	QDateTime date;
	if (dateTime.isValid())
		date=dateTime;
	else
		date = QDateTime::currentDateTime();
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"INSERT INTO conference_log(conference_id, datetime, private, nick_id,"
			" action_type, message, params, dst_nick_id)"
			" VALUES(?, ?, ?, ?, ?, ?, ?, ?)");
	q.addBindValue(nick->conference()->id());
	q.addBindValue(date);
	q.addBindValue(priv);
	q.addBindValue(nick->id());
	q.addBindValue(type);
	q.addBindValue(msg);
	q.addBindValue(params);
	q.addBindValue(dstNick ? dstNick->id() : QVariant());
	if (!q.exec())
	{
		qDebug() << "SQL: Unable to log event: " << q.lastError().text();
	}
}

QList<MucHistory::HistoryItem> MucHistory::missingHighlights(Nick *nick, QDateTime startDateTime) const
{
	QList<MucHistory::HistoryItem> res;
	if (!startDateTime.isValid())
		return res;

	Nick *botNick = nick->conference()->botNick();
	int botNickId = 0;
	if (botNick)
		botNickId = botNick->id();

	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"SELECT conference_log.datetime, conference_nicks.nick,"
			" conference_log.message FROM conference_log "
			" LEFT JOIN conference_nicks ON conference_nicks.id=conference_log.nick_id"
			" WHERE conference_log.private=false AND conference_log.conference_id=?"
			" AND conference_log.action_type=?"
			" AND conference_log.dst_nick_id=? AND conference_log.datetime>=?"
			" AND conference_log.nick_id!=?");
	q.addBindValue(nick->conference()->id());
	q.addBindValue(ActionMessage);
	q.addBindValue(nick->id());
	q.addBindValue(startDateTime);
	q.addBindValue(botNickId);
	if (!q.exec())
		return res;
	while (q.next())
	{
		MucHistory::HistoryItem item;
		item.dateTime = q.value(0).toDateTime();
		item.nick = q.value(1).toString();
		item.message = q.value(2).toString();
		res << item;
	}
	return res;
}
