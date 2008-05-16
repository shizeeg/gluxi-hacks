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
	myValidateRequired=false;
	myParent=parent;
	myNick=nick;
	myLazyLeave=false;
	myJoined=QDateTime::currentDateTime();
	qDebug() << "[NICK] created: " << myNick;

	myJid=new Jid(this, jid);

	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT id FROM conference_nicks WHERE conference_id = ? AND nick = ? AND jid = ?");
	query.addBindValue(myParent->id());
	query.addBindValue(myNick);
	query.addBindValue(myJid->id());
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


Nick::Nick(Conference* parent, int id)
{
	myParent=parent;
	myLazyLeave=false;
	myId=id;
	myValidateRequired=false;
		
	
	QSqlQuery query=DataStorage::instance()
			->prepareQuery("SELECT nick, jid, joined, lastaction FROM conference_nicks WHERE conference_id = ? AND id = ?");
	query.addBindValue(parent->id());
	query.addBindValue(id);
	if (!query.exec())
	{
		qDebug() << "Nick: " << QSqlDatabase::database().lastError().text();
		return;
	}
	if (!query.next())
	{
		qDebug() << "Nick: Unable to load nick with id=" << id;
		return;
	}
	myNick=query.value(0).toString();
	myJid=new Jid(this, query.value(1).toInt());
	myJidS=myJid->jid();
	myJoined=query.value(2).toDateTime();
	myLastActivity=query.value(3).toDateTime();
}

Nick::~Nick()
{
	if (!myLazyLeave)
	{
		qDebug() << "[NICK] destroyed: " << myNick;
		delete myJid;
		QSqlQuery query=DataStorage::instance()
			->prepareQuery("UPDATE conference_nicks SET online=false WHERE id=?");
		query.addBindValue(myId);
		query.exec();
	}
	else
	{
		delete myJid;
	}
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
	if (myValidateRequired)
		myValidateRequired=false;
	else
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

QStringList Nick::similarNicks()
{
	QSqlQuery query=DataStorage::instance()
			->prepareQuery("SELECT nick FROM conference_nicks WHERE conference_id=? and jid=? order by lastaction desc LIMIT 100");
	query.addBindValue(myParent->id());
	query.addBindValue(myJid->id());
	if (!query.exec())
	{
		qDebug() << "Nick: " << QSqlDatabase::database().lastError().text();
		return QStringList();
	}
	QStringList res;
	while (query.next())
	{
		res.append(query.value(0).toString());
	}
	return res;
}

void Nick::setAllOffline (Conference* conf)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conference_nicks SET online = false WHERE conference_id = ?");
	query.addBindValue(conf->id());
	query.exec();
	Jid::removeTemporary(conf);
}
