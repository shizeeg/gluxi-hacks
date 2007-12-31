#include "nick.h"
#include "conference.h"
#include "jid.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QVariant>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>

Nick::Nick(Conference* parent, const QString& nick, const QString& jid)
{
	myParent=parent;
	myNick=nick;
	myJoined=QDateTime::currentDateTime();
	qDebug() << "[NICK] created: " << myNick;

	myJid=new Jid(this, jid);

	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT id FROM conference_nicks WHERE conference_id = ? AND nick = ?");
	query.addBindValue(myParent->id());
	query.addBindValue(myNick);
	if (!query.exec())
	{
		qDebug() << "Nick::Nick: " << QSqlDatabase::database().lastError().text();
		return;
	}
	if (query.next())
	{
		myId=query.value(0).toInt();
		query.clear();
		query.prepare("UPDATE conference_nicks SET jid=?, joined = ?, lastaction = ?, online = true WHERE id = ?");
		query.addBindValue(myJid->id());
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(myId);
		query.exec();
	}
	else
	{
		query.clear();
		query.prepare("INSERT INTO conference_nicks ( conference_id, nick, jid, created, online, joined, lastaction ) "
			"VALUES ( ? , ? , ? , ? , ? , ? , ? )");
		query.addBindValue(parent->id());
		query.addBindValue(myNick);
		query.addBindValue(myJid->id());
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(true);
		query.addBindValue(QDateTime::currentDateTime());
		query.addBindValue(QDateTime::currentDateTime());
		query.exec();
		myId=query.lastInsertId().toInt();
	}
}


Nick::~Nick()
{
	qDebug() << "[NICK] destroyed: " << myNick;
	delete myJid;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conference_nicks SET online=false WHERE id=?");
	query.addBindValue(myId);
	query.exec();
}

void Nick::setNick(const QString& nick)
{
	if (myNick==nick) return;
	qDebug() << "[NICK] " << myNick << " is now known as " << nick;
	myNick=nick;
}

void Nick::updateLastActivity()
{
	qDebug() << "[NICK] Activity updated: " << myNick;
	myLastActivity=QDateTime::currentDateTime();
// 	commit();
};

void Nick::setJid(const QString& jid)
{
	myJidS=jid; //TODO: Deprecated, Remove this
	myJid->setFullJid(jid);
};

void Nick::commit()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conference_nicks SET nick=?, jid=?, lastaction=? WHERE id=?");
	query.addBindValue(myNick); // Nick
	query.addBindValue(myJid->id());
	query.addBindValue(myLastActivity);
	query.addBindValue(myId);
	query.exec();
}

void Nick::setAllOffline (Conference* conf)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conference_nicks SET online = false WHERE conference_id = ?");
	query.addBindValue(conf->id());
	query.exec();
	Jid::removeTemporary(conf);
}
