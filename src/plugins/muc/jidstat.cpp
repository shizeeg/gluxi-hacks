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
#include <QRegExp>

JidStat::JidStat(int jidId)
{
	id_ = 0;
	jidId_ = jidId;
	readOnly_ = false;

	dateTime_ = QDateTime::currentDateTime();
	if (jidId_ > 0)
	{
		if (!load())
			create();
	}
	else
	{
		readOnly_ = true;
	}
}

JidStat::~JidStat()
{
}

JidStat *JidStat::queryReadOnly(int jidId)
{
	JidStat *res = new JidStat(0);
	res->jidId_ = jidId;
	if (!res->load())
	{
		delete res;
		res = NULL;
	}
	return res;
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
		q.prepare(QString("UPDATE conference_jidstat set %1 = %2 + 1 WHERE id=?").arg(cntName).arg(cntName));
		q.addBindValue(id_);
		if (!q.exec())
		{
			qDebug() << "ERROR: Unable to update jidstat for field: " << cntName;
		}
	}
	updateOnlineTime();
}

void JidStat::setVersion(const QString& name, const QString& version, const QString& os)
{
	if (id_ <= 0)
		return;

	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"UPDATE conference_jidstat SET ver_name=?, ver_version=?,ver_os=? WHERE id=?"
	);
	q.addBindValue(name);
	q.addBindValue(version);
	q.addBindValue(os);
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to update version info";
	}
}

void JidStat::updateOnlineTime()
{
	if (id_ <= 0)
		return;
	QDateTime now = QDateTime::currentDateTime();
	int delta = dateTime_.secsTo(now);
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"UPDATE conference_jidstat SET time_online = time_online + ? WHERE id=?"
	);
	q.addBindValue(delta);
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to update time_online";
	}
	else
	{
		dateTime_=now;
	}

}

void JidStat::statMessage(const QString& message)
{
	if (id_ <= 0)
		return;
	QString msg = message.trimmed();
	int msgChars = msg.length();
	int msgWords = 0;
	int msgSentences = 0;
	int msgMe = 0;
	if (msg.toLower().startsWith("/me "))
		msgMe = 1;
	bool hasWord = false;
	for (int i = 0; i < msgChars; ++i )
	{
		QChar chr = msg[i];
		if (chr == '.' || chr == '?' || chr == '!')
		{
			if (hasWord)
			{
				++msgSentences;
				++msgWords;
			}
			hasWord = false;
			continue;
		}
		else if (chr.isSpace())
		{
			if (hasWord)
				++msgWords;
			hasWord = false;
			continue;
		}
		hasWord = true;
	}
	if (hasWord)
	{
		++msgWords;
		++msgSentences;
	}

	QSqlQuery q = DataStorage::instance()->prepareQuery(
		"UPDATE conference_jidstat SET msg_count = msg_count + 1,"
		" msg_chars = msg_chars + ?, msg_words = msg_words + ?,"
		" msg_sentences = msg_sentences + ?, msg_me = msg_me + ? WHERE id=?");
	q.addBindValue(msgChars);
	q.addBindValue(msgWords);
	q.addBindValue(msgSentences);
	q.addBindValue(msgMe);
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to stat sentence: " << q.lastError().text();
	}
}

void JidStat::statReply()
{
	if (id_ <= 0)
		return;

	QSqlQuery q = DataStorage::instance()->prepareQuery(
		"UPDATE conference_jidstat SET msg_reply = msg_reply + 1 WHERE id=?");
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to stat reply: " << q.lastError().text();
	}
}

void JidStat::statSubject(const QString& subject)
{
	Q_UNUSED(subject);
	if (id_ <= 0)
		return;

	QSqlQuery q = DataStorage::instance()->prepareQuery(
		"UPDATE conference_jidstat SET msg_subject = msg_subject + 1 WHERE id=?");
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to stat subject: " << q.lastError().text();
	}
}

JidStat::StatAction JidStat::lastAction() const
{
	StatAction res;
	res.type = ActionNone;

	if (id_ <= 0)
		return res;

	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"SELECT lastaction, lastreason FROM conference_jidstat WHERE id = ?");
	q.addBindValue(id_);
	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to query for lastAction: " << q.lastError().text();
		return res;
	}
	if (q.next())
	{
		res.type = static_cast<ActionType>(q.value(0).toInt());
		res.reason = q.value(1).toString();
	}
	return res;
}
