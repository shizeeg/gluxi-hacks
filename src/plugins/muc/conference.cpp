#include "conference.h"
#include "alist.h"

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
	QSqlQuery query;
	query.prepare("SELECT id, name FROM conferences WHERE name = ?");
	query.addBindValue(myName);
	query.exec();
	if (query.next())
	{
		myId=query.value(0).toInt();
		if (myNick.isEmpty())
			myNick=query.value(2).toString();


		query.clear();
		query.prepare("UPDATE conferences SET nick = ?, joined = ?, online = 1, autojoin=1 WHERE id = ?");
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
		query.addBindValue(1);
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
	QSqlQuery query;
	query.clear();
	query.prepare("UPDATE conferences SET online = 0 WHERE id = ?");
	query.addBindValue(myId);
	query.exec();
	delete myKick;
	delete myVisitor;
	delete myModerator;
}

QStringList Conference::autoJoinList()
{
	QSqlQuery query;
	query.prepare("SELECT name, nick FROM conferences WHERE autojoin=1");
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
	QSqlQuery query;
	query.prepare("UPDATE conferences SET autojoin=? WHERE id=?");
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
