#include "nick.h"
#include "conference.h"
#include "jid.h"
#include "jidstat.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QVariant>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>

Nick::Nick(Conference* parent, const QString& nick, const QString& jid)
{
	devoicedNoVCard_=false;
	myValidateRequired=false;
	myParent=parent;
	myJidStat = NULL;
	myNick=nick;
	myLazyLeave=false;
	myJoined=QDateTime::currentDateTime();
	versionStored_=false;
	vcardPhotoSize_=-1;
	qDebug() << "[NICK] created: " << myNick;

	myJid=new Jid(this, jid);

	if (myJid->id() >= 0 )
		myJidStat = new JidStat(myJid->id());

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
		if (myId==0)
		{
			// lastInsertId don't work. Probably postgreSQL
			query=DataStorage::instance()
				->prepareQuery("SELECT id FROM conference_nicks WHERE conference_id = ? AND nick = ? AND jid = ?");
			query.addBindValue(myParent->id());
			query.addBindValue(myNick);
			query.addBindValue(myJid->id());
			if (!query.exec() || !query.next())
			{
				qDebug() << "[NICK] " << QSqlDatabase::database().lastError().text();
				return;
			}
			myId=query.value(0).toInt();
		}
	}
}


Nick::Nick(Conference* parent, int id)
{
	devoicedNoVCard_=false;
	myParent=parent;
	myJidStat = NULL;
	myLazyLeave=false;
	myId=id;
	myValidateRequired=false;
	versionStored_=false;
	vcardPhotoSize_=-1;

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

	if (myJid->id() >= 0 )
			myJidStat = new JidStat(myJid->id());

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
	if (myJidStat)
		myJidStat->updateOnlineTime();
	delete myJidStat;
}

void Nick::setNick(const QString& nick)
{
	if (myNick==nick) return;
	qDebug() << "[NICK] " << myNick << " is now known as " << nick;
	myNick=nick;
}

void Nick::updateLastActivity()
{
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

QStringList Nick::nickToJids(Conference* conf, const QString& n, bool last)
{
	if(!conf)
		return QStringList();
	QStringList jids;

	Nick *nick=conf->nicks()->byName(n);

	if(!nick)
	{
		QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT conference_jids.jid FROM conference_nicks LEFT JOIN "
		"conference_jids ON conference_jids.id = conference_nicks.jid "
		"WHERE conference_nicks.conference_id=? AND conference_nicks.nick=? "
		"AND conference_jids.temporary = false ORDER BY conference_nicks.joined DESC LIMIT ?");
		query.addBindValue(conf->id());
		query.addBindValue(n);
		query.addBindValue((last) ? 1:3);

		if (!query.exec())
		{
			qDebug() << "Nick: " << QSqlDatabase::database().lastError().text();
			return QStringList();
		}
		while (query.next())
		{
			jids.append(query.value(0).toString());
		}
		return jids;
	}
	jids.append(nick->jidStr());
	return jids;

}

