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

#include "jidtimestat.h"

#include "base/datastorage.h"
#include "base/common.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QRegExp>
#include <QStringList>

struct RateReport
{
	QString name;
	QString title;
	QString fieldNames;
	QString fieldTitles;
	QString encType; // t -- time
};

static RateReport reportList[] =
{
	{
		"online",
		"Online time report",
		"conference_jidstat.time_online as sort_field",
		"Online time",
		"t"
	},
	{
		"message",
		"Message statistic report",
		"conference_jidstat.msg_count as sort_field|conference_jidstat.msg_words|conference_jidstat.msg_sentences|conference_jidstat.msg_me|conference_jidstat.msg_reply",
		"Messages|Words|Sentences|/me|Replies",
		""
	},
	{
		"replies",
		"Message replies statistic report",
		"conference_jidstat.msg_reply as sort_field|conference_jidstat.msg_count|conference_jidstat.msg_words|conference_jidstat.msg_sentences|conference_jidstat.msg_me",
		"Replies|Messages|Words|Sentences|/me",
		""
	},
	{
			"subject",
			"Subject changes report",
			"msg_subject as sort_field|time_online",
			"Subject changes|Online time",
			"|t"
	},
	{
			"version",
			"Version report",
			"ver_name as sort_field|ver_version|ver_os",
			"Client|Version|OS",
			""
	},
	{
			"join",
			"Join report",
			"cnt_join as sort_field|time_online",
			"Joins|Online time",
			"|t"
	},
	{
			"presence",
			"Presence report",
			"cnt_presence as sort_field|time_online|cnt_ban|cnt_kick|cnt_visitor|cnt_moderator",
			"Presences|Online time|Bans|Kicks|Visitors|Moderators",
			"|t"
	},
	{
			"nick",
			"Nick changes report",
			"cnt_nickchange as sort_field|time_online",
			"Nick changes|Online time",
			"|t"
	},
	{
			"visitor",
			"Visitor report",
			"cnt_visitor as sort_field|time_online",
			"Visitor|Online time",
			"|t"
	},
	{
			"participant",
			"Participant report",
			"cnt_participant as sort_field|time_online",
			"Participant|Online time",
			"|t"
	},
	{
			"moderator",
			"Moderator report",
			"cnt_moderator as sort_field|time_online",
			"Moderator|Online time",
			"|t"
	},
	{
			"kick",
			"Kick report",
			"cnt_kick as sort_field|time_online",
			"Kicks|Online time",
			"|t"
	},
	{
			"ban",
			"Ban report",
			"cnt_ban as sort_field|time_online",
			"Bans|Online time",
			"|t"
	},
	{"", "", "", ""}
};

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
		timeStat_ = new JidTimeStat(jidId_);
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

	if (timeStat_)
		timeStat_->logMessage(QDateTime::currentDateTime());

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
			"SELECT lastaction, lastreason, ver_name, ver_version, ver_os FROM conference_jidstat WHERE id = ?");
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
		res.verName = q.value(2).toString();
		res.verVersion = q.value(3).toString();
		res.verOs = q.value(4).toString();
	}
	return res;
}

QString JidStat::availableReports()
{
	QString res;
	RateReport *rep = &reportList[0];
	while(!rep->name.isEmpty())
	{
		res += QString("\n%1 -- %2").arg(rep->name, rep->title);
		++rep;
	}
	return QString("Available reports:%1").arg(res);
}

QString JidStat::queryReport(int conferenceId, const QString& type, int numRes)
{
	RateReport *rep = &reportList[0];
	while(!rep->name.isEmpty())
	{
		if (rep->name == type)
			break;
		++rep;
	}

	if (rep->name.isEmpty())
		return QString::null;

	QString fields = rep->fieldNames;
	fields.replace("|", ", ");
	QString qStr(
			"SELECT conference_nicks.nick, %1 from conference_jids"
			" JOIN conference_jidstat ON conference_jidstat.jid_id = conference_jids.id"
			" JOIN conference_nicks ON conference_nicks.jid = conference_jids.id"
			" AND conference_nicks.online = true"
			" WHERE conference_jids.conference_id = ? ORDER BY sort_field DESC limit ?");
	qStr = qStr.arg(fields);

	QSqlQuery q = DataStorage::instance()->prepareQuery(qStr);
	q.addBindValue(conferenceId);
	q.addBindValue(numRes);

	if (!q.exec())
	{
		qDebug() << "ERROR: Unable to execute query: " << q.lastError().text();
		qDebug() << "Query: " << qStr;
		return "Unable to prepare report";
	}

	int colNum = q.record().count();
	QVector<QVector<QString> > tbl;
	QVector<QString> head(colNum);
	head[0] = "Nick";
	for (int i = 1; i< colNum; ++i)
	{
		head[i] = rep->fieldTitles.section('|', i-1, i-1);
	}
	tbl << head;
	QString jidIdList;
	while (q.next())
	{
		QVector<QString> row(colNum);
		for (int i = 0; i < colNum; ++i)
		{
			QVariant v = q.value(i);
			QString enc = i > 0 ? rep->encType.section('|', i-1, i-1) : "";
			if (enc == "t")
				row[i] = secsToString(v.toInt());
			else
				row[i] = v.toString();
		}
		tbl << row;
		QString id = q.value(0).toString();
		if (!jidIdList.isEmpty())
			jidIdList += ", " + id;
		else
			jidIdList = id;
	}
	return formatTable(tbl);
}

QString JidStat::availableTimeReports()
{
	return JidTimeStat::reportList().join(", ");
}

QString JidStat::queryTimeReport(const QString& reportName)
{
	if (!timeStat_)
		return QString::null;
	return timeStat_->reportGraph(reportName);
}
