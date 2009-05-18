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
#include "jidstat.h"

#include "base/datastorage.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

JidStat::JidStat(int jidId)
{
	id_ = 0;
	jidId_ = jidId;

	if (jidId_ > 0)
	{
		if (!load())
			create();
	}
}

JidStat::~JidStat()
{
}

bool JidStat::load()
{
	id_ = 0;
	QSqlQuery q = DataStorage::instance() -> prepareQuery(
			"SELECT id FROM conference_jidstat WHERE jid_id=?"
	);
	q.addBindValue(jidId_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to query for jidstat id";
		return false;
	}
	if (!q.next())
		return false;
	id_ = q.value(0).toInt();
	return true;
}

void JidStat::create()
{
	id_ = 0;
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"INSERT INTO conference_jidstat(jid_id) VALUES (?)");
	q.addBindValue(jidId_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to add conference_jidstat entry" << q.lastError().text();
		return;
	}
	id_ = q.lastInsertId().toInt();
	if (id_ <= 0)
	{
		// PostgreSQL?
		if (!load())
		{
			qDebug() << "ERROR: Unable to load stored conference_jidstat entry";
		}
	}
}

void JidStat::commit()
{

}

void JidStat::setLastAction(ActionType type, const QString& reason)
{
	if (id_ <= 0)
		return;
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"UPDATE conference_jidstat SET lastaction=?, lastreason=? WHERE id=?"
	);
	q.addBindValue(type);
	q.addBindValue(reason);
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to update Last action: " << q.lastError().text();
	}

	QString cntName;
	switch (type)
	{
	case ActionJoin:
		cntName = "cnt_join";
		break;
	case ActionLeave:
		cntName = "cnt_leave";
		break;
	case ActionPresence:
		cntName = "cnt_presence";
		break;
	case ActionNickChange:
		cntName = "cnt_nickchange";
		break;
	case ActionVisitor:
		cntName = "cnt_visitor";
		break;
	case ActionParticipant:
		cntName = "cnt_participant";
		break;
	case ActionModerator:
		cntName = "cnt_moderator";
		break;
	case ActionNoAffiliation:
		cntName = "cnt_noaffiliation";
		break;
	case ActionMember:
		cntName = "cnt_member";
		break;
	case ActionAdministrator:
		cntName = "cnt_administrator";
		break;
	case ActionOwner:
		cntName = "cnt_owner";
		break;
	case ActionBan:
		cntName = "cnt_ban";
		break;
	case ActionKick:
		cntName = "cnt_kick";
		break;
	}

	if (!cntName.isEmpty())
	{
		q.prepare(QString("UPDATE conference_jidstat set %1 = %1 + 1 WHERE id=?").arg(cntName, cntName));
		q.addBindValue(id_);
		if (!q.exec())
		{
			qDebug() << "ERROR: Unable to update jidstat for field: " << cntName;
		}
	}
}
