#include "conference.h"
#include "alist.h"
#include "base/common.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDateTime>

Conference::Conference()
{
	qDebug() << "new Conference";
}

Conference::Conference(const QString& name, const QString& nick)
{
	myName=name;
	myNick=nick;
	qDebug() << myName;
	qDebug() << myNick;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT id, name FROM conferences WHERE name = ?");
	query.addBindValue(myName);
	query.exec();
	if (query.next())
	{
		myId=query.value(0).toInt();
		if (myNick.isEmpty())
			myNick=query.value(2).toString();


		query.clear();
		query.prepare("UPDATE conferences SET nick = ?, joined = ?, online = true, autojoin = true WHERE id = ?");
		query.addBindValue(myNick);
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(myId);
		if (!query.exec())
		{
			qWarning() <<"Unable to update nickname in database";
		}

	}
	else
	{
		query.clear();
		query.prepare("INSERT INTO conferences ( name , nick , created , online , joined ) VALUES ( ? , ? , ? , ? , ? )");
		query.addBindValue(myName);
		query.addBindValue(myNick);
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(true);
		query.addBindValue(QDateTime::currentDateTime());
		if (!query.exec())
		{
			qWarning() << "Unable to insert new Conference";
			return;
		}
		query.clear();
		query.prepare("SELECT id FROM conferences WHERE name = ?");
		qDebug() << myName;
		query.addBindValue(myName);
		query.exec();
		if (!query.next())
		{
			qDebug() << QSqlDatabase::database().lastError().text();
		}
		myId=query.value(0).toInt();

		query.clear();
		query.prepare("UPDATE conferences SET joined = ? WHERE id = ?");
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(myId);
	}
	Nick::setAllOffline(this);
	myKick=new AList(this,ALIST_KICK);
	myVisitor=new AList(this, ALIST_VISITOR);
	myModerator=new AList(this, ALIST_MODERATOR);
}

Conference::~Conference()
{
	qDebug() << "~Conference";
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET online = false WHERE id = ?");
	query.addBindValue(myId);
	query.exec();
	delete myKick;
	delete myVisitor;
	delete myModerator;
}

QStringList Conference::autoJoinList()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT name, nick FROM conferences WHERE autojoin=true");
	query.exec();
	QStringList result;
	while (query.next())
	{
		QString conf=query.value(0).toString();
		QString resource=query.value(1).toString();
		if (resource.isEmpty())
			result.append(conf);
		else
			result.append(conf+"/"+resource);
	}
	return result;
}

void Conference::setAutoJoin(bool b)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET autojoin=? WHERE id=?");
	query.addBindValue(b);
	query.addBindValue(myId);
	query.exec();
}

void Conference::removeExpired()
{
	myKick->removeExpired();
	myVisitor->removeExpired();
	myModerator->removeExpired();
}

QString Conference::seen(const QString&n)
{
	Nick* nick=myNicks.byName(n);
//	int cnt=0;
	if (nick)
	{
		return QString("\"%1\" is already in room (Joined %2 ago)").arg(n)
			.arg(secsToString(nick->joined().secsTo(QDateTime::currentDateTime())));
	}
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT jid FROM conference_nicks WHERE conference_id=? AND nick=?");
	query.addBindValue(myId);
	query.addBindValue(n);
	if (query.exec() && query.next())
	{
		int jid=query.value(0).toInt();
		query.prepare("SELECT online, nick, lastaction, joined FROM conference_nicks WHERE "
			"conference_id=? AND jid=? ORDER BY lastaction DESC LIMIT 1");
		query.addBindValue(myId);
		query.addBindValue(jid);
		if (query.exec() && query.next())
		{
			bool online=query.value(0).toBool();
			QString newNick=query.value(1).toString();
			QDateTime lastAction=query.value(2).toDateTime();
			QDateTime joinedTime=query.value(3).toDateTime();
			if (online)
				return QString("%1 is here with nick \"%2\" (Joined %3 ago)").arg(n).arg(newNick)
					.arg(secsToString(joinedTime.secsTo(QDateTime::currentDateTime())));
			QString secs=secsToString(lastAction.secsTo(QDateTime::currentDateTime()));
			if (newNick==n)
				return QString("%1 was here %2 ago").arg(n).arg(secs);
			return QString("%1 was here %2 ago with nick \"%3\"").arg(n).arg(secs).arg(newNick);
		}
	}
	return QString("I never see \"%1\" here").arg(n);
}

QString Conference::clientStat()
{
	QSqlQuery query;
	query.prepare("SELECT resource, COUNT(id) AS count FROM conference_jids WHERE conference_id=? "
		"GROUP BY resource ORDER BY count DESC LIMIT 20");
	query.addBindValue(myId);
	int idx=0;
	if (query.exec())
	{
		QString res;
		while (query.next())
			res+=QString("\n%1) %2: %3").arg(++idx).arg(query.value(0).toString())
				.arg(query.value(1).toInt());
		return QString("Client stats: %1").arg(res);
	}
	else
		return QString::null;
}

void Conference::setNick(const QString& nick)
{
	myNick=nick;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conferences SET nick=? WHERE conference_id=?");
	query.addBindValue(nick);
	query.addBindValue(myId);
	query.exec();
}

