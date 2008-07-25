#include "jid.h"
#include "conference.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QString>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>

Jid::Jid(Nick *parent, const QString& jid)
{
	myParent=parent;

	if (!jid.isEmpty())
	{
		myJid=jid.section('/',0,0);
		myResource=jid.section('/',1);
	}

	if (myJid.isEmpty())
	{
		myJid=QString("%1@").arg(parent->nick());
		myTemporary=true;
	}
	else
		myTemporary=false;

	loadJid();
}

Jid::Jid(Nick* parent, int id)
{
	myParent=parent;
	myId=id;
	myTemporary=false;
	QSqlQuery query=DataStorage::instance()
			->prepareQuery("SELECT jid,created FROM conference_jids WHERE conference_id = ? AND id = ?");
	query.addBindValue(parent->conference()->id());
	query.addBindValue(id);
	if (!query.exec() || !query.next())
	{
		qDebug() << "Unable to load JID id=" << id;
		return;
	}
	myJid=query.value(0).toString();
	myCreated=query.value(1).toDateTime();
}

Jid::~Jid()
{
// 	saveJid();
}

void Jid::loadJid()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT id,created FROM conference_jids WHERE conference_id = ? AND jid = ?");
	qDebug() << myParent->conference()->id() << myJid;
	query.addBindValue(myParent->conference()->id());
	query.addBindValue(myJid);
	if (!query.exec())
	{
		qDebug() << "[JID] " << QSqlDatabase::database().lastError().text();
		return;
	}
	if (query.next())
	{
		myId=query.value(0).toInt();
		myCreated=query.value(1).toDateTime();
	}
	else
	{
		myCreated=QDateTime::currentDateTime();
		query=DataStorage::instance()
			->prepareQuery("INSERT INTO conference_jids ( conference_id, jid, resource, temporary, created) "
			"VALUES ( ?, ?, ?, ?, ? )");
		query.addBindValue(myParent->conference()->id());
		query.addBindValue(myJid);
		query.addBindValue(myResource);
		query.addBindValue(myTemporary);
		query.addBindValue(myCreated);
		if  (!query.exec())
		{
			qDebug() << "[JID] " << QSqlDatabase::database().lastError().text();
		}
		myId=query.lastInsertId().toInt();
		if (myId==0)
		{
			// lastInsertId don't work. Probably postgreSQL
			query=DataStorage::instance()
				->prepareQuery("SELECT id FROM conference_jids WHERE conference_id=? AND jid=?");
			query.addBindValue(myParent->conference()->id());
			query.addBindValue(myJid);
			if (!query.exec() || !query.next())
			{
				qDebug() << "[JID] " << QSqlDatabase::database().lastError().text();
				return;
			}
			myId=query.value(0).toInt();
		}
	}

}

void Jid::setFullJid(const QString& fullJid)
{
	// Changed JID from emtpy to empty
	if (myTemporary && fullJid.isEmpty())
		return;

	if (fullJid.toUpper()==QString(myJid+"/"+myResource).toUpper())
		return;

	if (myTemporary)
	{
		// Changed from unknown to known
		remove();
	}
	else
	{
//	qWarning() << "[JID] Changin JID from " << myJid << " to unknown.";
		return;
	}

	myJid=fullJid.section('/',0,0);
	myResource=fullJid.section('/',1);
	if (myId>0)
	{
		updateJid();
	}
	else
	{
		loadJid(); // myId will be changed;
	}
}

void Jid::updateJid()
{
	if (myId<=0)
		return;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("UPDATE conference_jids SET jid=?, resource=? WHERE id=?");
	query.addBindValue(myJid);
	query.addBindValue(myResource);
	query.addBindValue(myId);
	query.exec();
}

void Jid::removeTemporary(Conference *conf)
{
	QSqlQuery query;
	if (conf)
	{
		query=DataStorage::instance()
			->prepareQuery("DELETE FROM conference_jids WHERE conference_id=? AND temporary=true");
		query.addBindValue(conf->id());
	}
	else
		query=DataStorage::instance()
			->prepareQuery("DELETE FROM conference_jids WHERE temporary=true");
	query.exec();
}

void Jid::remove()
{
		QSqlQuery query;
		query=DataStorage::instance()
			->prepareQuery("DELETE FROM conference_jids WHERE id = ?");
		query.addBindValue(myId);
		query.exec();
}

void Jid::commit()
{

}
