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
#include <QCoreApplication>
#include <QTextStream>
#include <QStringList>
#include <QSet>
#include <QtSql>

#error This is very ugly. Please read code first

QTextStream stream(stdout);

#define TARGET_PGSQL

bool exec(QSqlDatabase& db, const QString& query)
{
	QSqlQuery q(db);
	if (!q.exec(query)) {
		stream << "Unable to execute: " << q.lastError().text() << endl;
		return false;
	}
	return true;
}


int main(int argc, char *argv[])
{
	QStringList tables;
	QStringList safeErrors;

	safeErrors << "conference_alists_conference_id_key";
	safeErrors << "insert or update on table \"conference_jids\" violates foreign key constraint \"conference_jids_conference_id_fkey\"";
	safeErrors << "conference_nicks_jid_fkey";
	safeErrors << "conference_nicks_conference_id_fkey";


	QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
	QTextCodec::setCodecForCStrings(QTextCodec::codecForName("utf8"));

	tables << "acl" << "aliases";
	tables << "conferences";
	tables << "conference_alists";
	tables << "conference_jids";
	tables << "conference_nicks";
	tables << "webstatus";
	tables << "words";
	tables << "configuration";
	tables << "configuration_fields";
	tables << "roster";

	QSet<QString> forcedBool;
	forcedBool << "aliases.global"
		<< "conferences.autojoin" << "conferences.online" << "conferences.autoleave"
		<< "conference_alists.inv" << "conference_jids.temporary" << "conference_nicks.online";

	QCoreApplication app(argc, argv);
#ifdef CONVERT_DB
	QSqlDatabase dbSrc = QSqlDatabase::addDatabase("QMYSQL", "src");
	dbSrc.setHostName("localhost");
	dbSrc.setDatabaseName("gluxi");
	dbSrc.setUserName("root");
	dbSrc.setPassword("root");

	if (dbSrc.open()) {
		stream << "Connected to source database" << endl;
	} else {
		stream << "Unable to open source database: " <<
				dbSrc.lastError().text() << endl;
		return 1;
	}
#endif

	QSqlDatabase dbDst = QSqlDatabase::addDatabase("QPSQL7", "dst");
	dbDst.setHostName("localhost");
	dbDst.setDatabaseName("gluxi");
	dbDst.setUserName("gluxi");
	dbDst.setPassword("gluxi");

	if (dbDst.open()) {
		stream << "Connected to destination database" << endl;
	} else {
		stream << "Unable to open destination database: " <<
		dbDst.lastError().text() << endl;
		return 1;
	}

#ifdef CONVERT_DB
	for(QStringList::iterator it = tables.begin(); it != tables.end(); ++it) {
		QString tbl = *it;
		stream << "==> Converting: " << tbl << endl;


		if (exec(dbDst, QString("TRUNCATE %1 CASCADE").arg(tbl)))
			stream << "Destination table truncated" << endl;
		else
			return 1;


		QSqlQuery countQuery(dbSrc);
		if (!countQuery.exec(QString("SELECT COUNT(*) FROM %1").arg(tbl))
				|| !countQuery.next()) {
			stream << "Unable to query record count: " << countQuery.lastError().text();
			return 1;
		}

		int recCount = countQuery.value(0).toInt();

		stream << "Record count: " << recCount << endl;

		int recNum = 0;
		int numErr = 0;
#if 0
		while (recNum < recCount)
		{
#endif
			QSqlQuery srcQuery(dbSrc);
			srcQuery.setForwardOnly(true);
			if (!srcQuery.exec(QString("SELECT * FROM %1").arg(tbl))) {
				stream << "Unable to query records: " << srcQuery.lastError().text();
				return 1;
			}

#ifdef TARGET_PGSQL
			bool hasSequence = false;
#endif

			QSqlRecord rec = srcQuery.record();
			QString insertStr = QString ("INSERT INTO %1 (").arg(tbl);
			QString valueStr = "";
			for (int i = 0; i < rec.count(); ++i) {
				if (i != 0) {
					insertStr += ", ";
					valueStr += ", ";
				}
				insertStr += rec.fieldName(i);
#ifdef TARGET_PGSQL
				if (rec.fieldName(i) == "id")
					hasSequence = true;
#endif
				valueStr += "?";
			}
			insertStr += ") VALUES (" + valueStr+ ")";

			stream << "" << insertStr << endl;

			QSqlQuery dstQuery(dbDst);
#if 1
			if (!dstQuery.prepare(insertStr)) {
				stream << "Unable to prepare dst query" << endl;
				return 1;
			}
#endif
			while (srcQuery.next()) {
				int cnt = srcQuery.record().count();
				for (int i = 0; i < cnt; ++i) {
					QVariant v = srcQuery.value(i);
					if (forcedBool.contains(QString("%1.%2").arg(tbl, srcQuery.record().fieldName(i)))) {
						v = v.toInt() ? QVariant(true) : QVariant(false);
					}
					if (v.type()==QVariant::ByteArray)
						v = QString(v.toByteArray());
					dstQuery.bindValue(i, v);
				}
#if 1
				if (!dstQuery.exec()) {
					stream << "   record: " << recNum << endl;
					QString errText = dstQuery.lastError().text();
					bool safe = false;
					foreach(QString safeErr, safeErrors) {
						if (errText.contains(safeErr)) {
							safe = true;
							break;
						}
					}

					stream << "==> Unable to exec insert: " << dstQuery.lastError().text() << endl;
					int cnt = srcQuery.record().count();
					for (int i = 0; i < cnt; ++i) {
						QVariant v = srcQuery.value(i);
						stream << "==> " << srcQuery.record().fieldName(i) << ": " << v.toString() << endl;
					}
					if (!safe) {
						return 1;
					} else {
						++numErr;
					}
				}
#endif
				stream << "  processed record " << recNum << endl;
				++recNum;
			}
#if 0
		}
#endif
		stream << "Done, " << recNum << " records" << ", Errors: " << numErr << endl;

#ifdef TARGET_PGSQL
		if (hasSequence)
		{
			stream << "Setting new value for sequence, tbl=" << tbl << endl;

			QSqlQuery maxValueQuery(dbDst);
			if (!maxValueQuery.exec(QString("SELECT max(id) FROM %1").arg(tbl))
					|| !maxValueQuery.next()) {
				stream << "Unable to get max value for SERIAL" << endl;
				return 1;
			}
			int curValue = maxValueQuery.value(0).toInt();
			stream << "... curValue=" << curValue << endl;
			++curValue;
			if (!maxValueQuery.exec(QString(
					"alter sequence %1_id_seq restart with %2").arg(tbl).arg(curValue)))
			{
				stream << "Unable to alter sequence value: " << maxValueQuery.lastError().text() << endl;
				return 1;
			}
		}
#endif
	}
#else

	stream << "Removing duplicate JID records" << endl;
	QSqlQuery badCaseQuery(dbDst);
	if (!badCaseQuery.exec("select conference_id, jid from (select conference_id, jid, count(jid) as cnt from conference_jids group by conference_id, jid) as tbl where cnt > 1"))
	{
		stream << "Unable to query for bad case JIDs" << endl;
		return 1;
	}
	while (badCaseQuery.next()) {
		int conference_id = badCaseQuery.value(0).toInt();
		QString jid = badCaseQuery.value(1).toString();
		stream << "conference_id: " << conference_id << ", jid=" << jid << endl;

		QSqlQuery bestJidQuery(dbDst);
		bestJidQuery.prepare("select id from conference_jids where conference_id=? and jid=? order by created limit 1");
		bestJidQuery.addBindValue(conference_id);
		bestJidQuery.addBindValue(jid);
		if (!bestJidQuery.exec() || !bestJidQuery.next()) {
			stream << "Unable to query for best jid" << endl;
			return 1;
		}
		int bestJidId = bestJidQuery.value(0).toInt();
		stream << " best jid id: " << bestJidId << endl;
		QSqlQuery fixQuery(dbDst);
		fixQuery.prepare(
				"UPDATE conference_nicks set jid=? where conference_id=? and jid in ("
				"select id from conference_jids where conference_id=? and jid=?)");
		fixQuery.addBindValue(bestJidId);
		fixQuery.addBindValue(conference_id);
		fixQuery.addBindValue(conference_id);
		fixQuery.addBindValue(jid);
		if (!fixQuery.exec()) {
			stream << "Unable to fix nicks" << endl;
			return 1;
		}
		stream << " fixed nicks: " << fixQuery.numRowsAffected() << endl;

		fixQuery.prepare("DELETE from conference_jids where conference_id=? and jid=? and id<>?");
		fixQuery.addBindValue(conference_id);
		fixQuery.addBindValue(jid);
		fixQuery.addBindValue(bestJidId);
		if (!fixQuery.exec()) {
			stream << "Unable to remove extra jids" << endl;
			return 1;
		}
		stream << " removed jids: " << fixQuery.numRowsAffected() << endl;
	}

	stream << "Removing duplicate nick records" << endl;

	QSqlQuery dublNickQuery(dbDst);
	if (!dublNickQuery.exec("select * from (select conference_id, nick, jid, count(jid) from conference_nicks group by conference_id, nick, jid) as tbl where count > 1"))
	{
		stream << "Unable to query for bad case JIDs" << endl;
		return 1;
	}
	while (dublNickQuery.next()) {
		int conference_id = dublNickQuery.value(0).toInt();
		QString nick = dublNickQuery.value(1).toString();
		int jid_id = dublNickQuery.value(2).toInt();

		stream << "conference_id: " << conference_id << ", nick:" << nick << ", jid_id:" << jid_id << endl;
		QSqlQuery fixQuery(dbDst);
		fixQuery.prepare(
				"delete from conference_nicks where conference_id=? and jid=? and nick=? and not id in ("
				"select id from conference_nicks where conference_id=? and jid=? and nick=? order by created LIMIT 1)");
		fixQuery.addBindValue(conference_id);
		fixQuery.addBindValue(jid_id);
		fixQuery.addBindValue(nick);
		fixQuery.addBindValue(conference_id);
		fixQuery.addBindValue(jid_id);
		fixQuery.addBindValue(nick);

		if (!fixQuery.exec())
		{
			stream << "Unable to remove duplicate nicks";
		}
		stream << " removed nicks: " << fixQuery.numRowsAffected() << endl;
	}

#endif
}
