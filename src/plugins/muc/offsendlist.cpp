#include "offsendlist.h"
#include "base/datastorage.h"

#include <QList>
#include <QVariant>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QtDebug>

#define MAXOLD 25

OffsendList::OffsendList()
{
}

QString OffsendList::list(const StorageKey& storage, const Nick* nick)
{
	if (!storage.isValid())
		return QString::null;

	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT sent, name, nick, message FROM offsend WHERE "
		"storage=? AND name=? ORDER BY date DESC");
	query.addBindValue(storage.storage());
	query.addBindValue(nick->nick());
	if (!query.exec())
		return "Can't get message list!";

	QString msg;
	int i = 0;
	while (query.next())
	{
		msg += QString("\n%1) %2 %3 to %4: %5")
		  .arg(++i)
		  .arg(query.value(0).toInt() == 0 ? "P":"S")
		  .arg(query.value(1).toString())
		  .arg(query.value(2).toString())
		  .arg(query.value(3).toString().left(17))
		  += (query.value(3).toString().length() > 17) ? "...":"";
	}
	return (msg.isEmpty()) ? "No messages" : msg;
}

QStringList OffsendList::getJids(const StorageKey& storage)
{
	if (!storage.isValid())
		return QStringList();

	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT jid FROM offsend WHERE sent=0");
			       //" WHERE storage=?");
	//query.addBindValue(storage.storage());
	query.exec();
	_jids.clear();
	while (query.next())
		_jids.append(jidToString(storage, query.value(0).toInt()));
	return _jids;
}

bool OffsendList::hasMessage(const QString& jid)
{
  // qDebug() << "Offsend::hasMessage(const QString& jid): jid = " << jid << "jids = " << _jids.join(" ") << " " << (_jids.contains(jid) == true);
	return _jids.contains(jid);
}

QString OffsendList::get(const StorageKey& storage, const Nick* nick, int count)
{
	if (!storage.isValid())
		return QString::null;

	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT id, name, date, message FROM offsend WHERE "
		"storage=? AND jid=? and sent=0 ORDER BY date DESC LIMIT ?");
	query.addBindValue(storage.storage());
	query.addBindValue(nick->jid()->id());
	query.addBindValue(count);
	query.exec();
	
	QStringList list;
	while (query.next())
	{
		list << QString("\nFrom: %1 (%2)\nMessage: %3")
			.arg(query.value(1).toString())
			.arg(query.value(2).toDateTime()
			     .toString("ddd, MMM dd HH:mm:ss yyyy"))
			.arg(query.value(3).toString());

		int id = query.value(0).toInt();
		query.clear();
		query.prepare("UPDATE offsend SET sent=? WHERE id=?");
		query.addBindValue(1);
		query.addBindValue(id);
		if (!query.exec())
			qWarning() << "Unable to update 'offsend' database!";
	}
	return list.isEmpty() ? QString::null : list.join("\n");
}

QMap<QString,QString> OffsendList::getAll(const StorageKey& storage, const QString& name)
{
	QMap<QString,QString> map;
	if (!storage.isValid())
		return map;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT nick, message FROM words WHERE "
			       "storage=? AND name=? ORDER BY date DESC");
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	if (!query.exec())
		return map;
	while (query.next())
		map[query.value(0).toString()]=query.value(1).toString(); // FIX IT!!!
	return map;
}

int OffsendList::append(const StorageKey& storage, const QString& name, const QString& nick, const QString& value)
{
	if (!storage.isValid())
		return 0;

	QSqlQuery query=DataStorage::instance()->prepareQuery(
	    "SELECT jid FROM conference_nicks WHERE "
	    "conference_id=? AND nick=? ORDER BY lastaction DESC LIMIT 1");
	query.addBindValue(storage.storage());
	query.addBindValue(nick);
	if (!query.exec() || !query.next())
		return -3; // no such nick ever appears in conference.
	int jid = query.value(0).toInt();
	qDebug() << "JID(int): " << jid << " " << nick;
	query.clear();
	query.prepare("INSERT INTO "
		      "offsend(storage, name, nick, jid, date, message, sent) "
		      "VALUES (?, ?, ?, ?, ?, ?, ?)");
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.addBindValue(nick);
	query.addBindValue(jid);
	query.addBindValue(QDateTime::currentDateTime());
	query.addBindValue(value);
	query.addBindValue(0);
	if (!query.exec())
	{
		// So we will try to update
		query.clear();
		query.prepare("UPDATE offsend SET value=? WHERE "
			      "storage=? AND name=? AND nick=? AND sent=?");
		query.addBindValue(value);
		query.addBindValue(storage.storage());
		query.addBindValue(name);
		query.addBindValue(nick);
		query.addBindValue(0);
		if (!query.exec())
			return 0;	
		return 2;
	}
	query.clear();
	query.prepare("SELECT COUNT(name) FROM offsend WHERE "
		      "storage=? AND name=?");
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	if (!query.exec())
	{
		qDebug() << "Can't get name count";
		return 0;
	}
	query.next();
	int totalCnt=query.value(0).toInt();
	int toDel=totalCnt-MAXOLD;
	if (toDel>0)
	{
		query.clear();
		query.prepare("DELETE FROM offsend WHERE storage=? AND name=? ORDER BY DATE LIMIT ?");
		query.addBindValue(storage.storage());
		query.addBindValue(name);
		query.addBindValue(toDel);
		query.exec();
	}
	return 1;
}

bool OffsendList::remove(const StorageKey& storage, const Nick *nick, int ndx)
{
	if (!storage.isValid())
		return false;
	int id = ndxToId(storage, nick, ndx);
	if (!id)
		return false;
	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"DELETE FROM offsend WHERE storage=? AND id=?");
	query.addBindValue(storage.storage());
	query.addBindValue(id);
	query.exec();
	return query.numRowsAffected();
}

QString OffsendList::jidToString(const StorageKey& storage, int jid)
{
	QSqlQuery query=DataStorage::instance()->prepareQuery(
	          "SELECT jid FROM conference_jids WHERE "
		  "conference_id=? AND id=? LIMIT 1");
	query.addBindValue(storage.storage());
	query.addBindValue(jid);
	if (!query.exec() || !query.next())
		return QString::null;
	return query.value(0).toString();
}

int OffsendList::stringToJid(const StorageKey& storage, const QString& jid)
{
	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT id FROM conference_jids WHERE "
		"conference_id=? AND jid=? LIMIT 1");
	query.addBindValue(storage.storage());
	query.addBindValue(jid);
	if (!query.exec() || !query.next())
		return 0;
	return query.value(0).toInt();
}

void OffsendList::clear(const StorageKey& storage, const Nick* nick)
{
	if (!storage.isValid())
		return;
	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"DELETE FROM offsend WHERE storage=? AND name=? OR jid=?");
	query.addBindValue(storage.storage());
	query.addBindValue(nick->nick());
	query.addBindValue(nick->jid());
	query.exec();
}

QString OffsendList::at(const StorageKey& storage, const Nick* nick, int ndx)
{
	if (!storage.isValid())
		return QString::null;
	int id = ndxToId(storage, nick, ndx);
	if (!id)
		return "Message ID not found!";

	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT message FROM offsend WHERE storage=? AND id=? LIMIT 1");
	query.addBindValue(storage.storage());
	query.addBindValue(id);
	if (!query.exec() || !query.next())
		return "Can't select message from database!";
	return query.value(0).toString();
}

int OffsendList::ndxToId(const StorageKey& storage, const Nick* nick, int index)
{
	if (!storage.isValid())
		return -1;
	// SELECT all messages from "nick" and to "nick".
	QSqlQuery query=DataStorage::instance()->prepareQuery(
		"SELECT id FROM offsend WHERE storage=? AND name=? OR jid=? "
		"ORDER BY date DESC");
	query.addBindValue(storage.storage());
	query.addBindValue(nick->nick());
	query.addBindValue(nick->id());
	if (!query.exec()) 
	{
		qDebug() << "Can't execute SQL.";
		return -2;
	}
	if (index)
		query.seek(index);
	else if (!query.next())
		return -3;
	return query.value(0).toInt();
}
