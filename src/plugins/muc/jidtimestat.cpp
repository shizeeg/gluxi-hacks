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
#include "jidtimestat.h"

#include "actiontype.h"

#include "base/common.h"
#include "base/datastorage.h"

#include <QDebug>
#include <QDateTime>
#include <QDate>
#include <QTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QVector>
#include <QStringList>

const static long MaxSecond = 7 * 24 * 60 * 60;

const static long ReportWidth = 20;

struct Report
{
	int idx;
	QString name;
	int div;
	long period;
	QString captions;
};


Report reportArray[] =
{
		{
			1,
			"hour",
			24 * 60 * 60,
			60 * 60,
			"00:00|01:00|02:00|03:00|04:00|05:00|06:00|07:00|08:00|09:00|10:00|11:00|12:00|"
					"13:00|14:00|15:00|16:00|17:00|18:00|19:00|20:00|21:00|22:00|23:00"
		},
		{
			2,
			"day/8",
			24 * 60 * 60,
			3 * 60 * 60,
			"00:00 -- 02:59|03:00 -- 05:59|06:00 -- 08:59|09:00 -- 11:59|"
			"12:00 -- 14:59|15:00 -- 17:59|18:00 -- 20:59|21:00 -- 23:59"
		},
		{
			3,
			"day/6",
			24 * 60 * 60,
			4 * 60 * 60,
			"00:00 -- 03:59|04:00 -- 07:59|08:00 -- 11:59|12:00 -- 15:59|16:00 -- 19:59|20:00 -- 23:59"
		},
		{
			4,
			"day",
			7 * 24 * 60 *60,
			24 * 60 * 60,
			"Mon|Tue|Wed|Thu|Fri|Sat|Sun"
		},
		{ 0, "", 0, 0, "" }
};

JidTimeStat::JidTimeStat(int jidId)
{
	jidId_ = jidId;
	if (!hasRecords())
	{
		createRecords();
		// Don't import LOG. Now it takes too much time
		// importLog();
	}
}

JidTimeStat::~JidTimeStat()
{
}

void JidTimeStat::logMessage(const QDateTime& dateTime)
{
	if (jidId_ <= 0)
		return;

	QTime time = dateTime.time();
	long secs = (((dateTime.date().dayOfWeek()-1)*24 + time.hour())*60L
			+ time.minute())*60 + time.second();
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"UPDATE conference_jidstat_time SET value=value+1"
			" WHERE jid_id=? AND int_type=? AND int_idx=?");
	for (int i=0;; ++i)
	{
		Report report = reportArray[i];
		if (!report.period)
			break;
		int idx = (secs % report.div ) / report.period;
//		qDebug() << "Updating SQL: " << dateTime << " secs=" << secs << secs % MaxSecond << "period=" << report.period
//			<< " | " << jidId_ << report.idx << idx;
		q.addBindValue(jidId_);
		q.addBindValue(report.idx);
		q.addBindValue(idx);
		if (!q.exec())
		{
			qDebug() << "[SQL] " << q.lastError().text();
		}
	}
}

bool JidTimeStat::hasRecords()
{
	if (jidId_ <= 0)
		return false;

	static int correctRecordCount = 0;

	if (!correctRecordCount)
	{
		for (int i=0;; ++i)
		{
			Report report = reportArray[i];
			if (!report.period)
				break;
			correctRecordCount += report.div/report.period;
			if (report.captions.count('|') != report.div/report.period-1)
			{
				qDebug() << "Looks like incorrect captions for report: " << report.idx << report.captions;
				qDebug() << "Fields: " << report.captions.count('|')+1 << ", but should be " << report.div/report.period;
				abort();
			}
		}
	}

	QSqlQuery q = DataStorage::instance()->prepareQuery(
				"SELECT COUNT(*) FROM conference_jidstat_time WHERE jid_id=?");
	q.addBindValue(jidId_);
	if (!q.exec() || !q.next())
		return false;
	int curRecordCount = q.value(0).toInt();
	if (curRecordCount != correctRecordCount)
		return false;
	return true;
}

void JidTimeStat::clearRecords()
{
	if (jidId_ <= 0)
		return;
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"DELETE FROM conference_jidstat_time WHERE jid_id=?");
	q.addBindValue(jidId_);
	if (!q.exec())
	{
		qDebug() << "[SQL] " << q.lastError().text();
	}
}

void JidTimeStat::createRecords()
{
	clearRecords();
	QSqlQuery q = DataStorage::instance()->prepareQuery(
			"INSERT INTO conference_jidstat_time(jid_id, int_type, int_idx) VALUES(?, ?, ?)");
	for (int i=0;; ++i)
	{
		Report report = reportArray[i];
		if (!report.period)
			break;
		int numIndexes = report.div / report.period;
		for (int idx = 0; idx < numIndexes; ++idx)
		{
			q.addBindValue(jidId_);
			q.addBindValue(report.idx);
			q.addBindValue(idx);
			if (!q.exec())
			{
				qDebug() << "[SQL] " << q.lastError().text();
				clearRecords();
				return;
			}
		}
	}
}

void JidTimeStat::importLog()
{
	if (jidId_ <= 0)
		return;
	qDebug() << "Importing message times for jid_id=" << jidId_;
	QSqlQuery srcQ = DataStorage::instance()->prepareQuery(
			"SELECT conference_log.datetime FROM conference_log"
			" LEFT JOIN conference_nicks ON conference_nicks.id=conference_log.nick_id"
			" WHERE conference_log.action_type=? and conference_nicks.jid=?");
	srcQ.setForwardOnly(true);
	srcQ.addBindValue(ActionMessage);
	srcQ.addBindValue(jidId_);
	if (!srcQ.exec())
	{
		qDebug() << "[SQL] Unable to query for chatlog: " << srcQ.lastError().text();
	}

	while (srcQ.next())
	{
		QDateTime dt = srcQ.value(0).toDateTime();
		logMessage(dt);
	}
}

QStringList JidTimeStat::reportList()
{
	QStringList res;
	for (int i=0;; ++i)
	{
		Report report = reportArray[i];
		if (!report.period)
			break;
		res << report.name;
	}
	return res;
}

QString JidTimeStat::reportGraph(const QString& reportName)
{
	for (int i=0;; ++i)
	{
		Report report = reportArray[i];
		if (!report.period)
			break;
		if (report.name.toLower() != reportName.toLower())
			continue;

		// Found report;
		QSqlQuery q = DataStorage::instance()->prepareQuery(
				"SELECT value FROM conference_jidstat_time"
				" WHERE jid_id=? AND int_type=? ORDER BY int_idx");
		q.addBindValue(jidId_);
		q.addBindValue(report.idx);
		if (!q.exec())
		{
			qDebug() << "[SQL] Unable to prepare report: " << q.lastError().text();
			return QString::null;
		}

		QVector<int> values;
		int max = 0;
		while (q.next())
		{
			int val = q.value(0).toInt();
			values << val;
			if (max < val)
				max = val;
		}

		if (max == 0)
			max = 1;

		QVector<QVector<QString> > tbl;
		int idx=0;
		foreach(int val, values)
		{
			QVector<QString> row(2);
			row[0] = report.captions.section('|', idx, idx);
			row[1] = QString("|%1 (%2)").arg(QString(ReportWidth * val / max, '#'), QString::number(val));
			++idx;
			tbl << row;
		}
		return formatTable(tbl);
	}
	return QString::null;
}
