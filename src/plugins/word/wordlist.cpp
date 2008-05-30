#include "wordlist.h"
#include "base/datastorage.h"

#include <QList>
#include <QVariant>
#include <QSqlQuery>
#include <QDateTime>
#include <QtDebug>

#define MAXOLD 5

WordList::WordList()
{
}

int WordList::append(const StorageKey& storage, const QString& name, const QString& nick, const QString& value)
{
	if (!storage.isValid())
		return 0;
	
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("INSERT INTO words(plugin, storage, name, date, nick, value) VALUES (?, ?, ?, ?, ?, ?)");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.addBindValue(QDateTime::currentDateTime());
	query.addBindValue(nick);
	query.addBindValue(value);
	if (!query.exec())
	{
		// So we will try to update
		query.clear();
		query.prepare("UPDATE words SET value=? WHERE plugin=? AND storage=? AND name=? AND nick=?");
		query.addBindValue(value);
		query.addBindValue(storage.plugin());
		query.addBindValue(storage.storage());
		query.addBindValue(name);
		query.addBindValue(nick);
		if (!query.exec())
			return 0;	
		return 2;
	}
	query.clear();
	query.prepare("SELECT COUNT(name) FROM words WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage.plugin());
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
		query.prepare("DELETE FROM words WHERE plugin=? AND storage=? AND name=? ORDER BY DATE LIMIT ?");
		query.addBindValue(storage.plugin());
		query.addBindValue(storage.storage());
		query.addBindValue(name);
		query.addBindValue(toDel);
		query.exec();
	}
	return 1;
}

QMap<QString,QString> WordList::getAll(const StorageKey& storage, const QString& name)
{
	QMap<QString,QString> map;
	if (!storage.isValid())
		return map;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT nick, value FROM words WHERE plugin=? AND storage=? AND name=? ORDER BY date DESC");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	if (!query.exec())
		return map;
	while (query.next())
		map[query.value(0).toString()]=query.value(1).toString();
	return map;
}

QString WordList::get(const StorageKey& storage, const QString&name, QString* nick)
{
	if (!storage.isValid())
		return QString::null;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT nick, value FROM words WHERE plugin=? AND storage=? AND name=? ORDER BY date DESC LIMIT 1");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.exec();
	if (!query.next())
		return QString::null;
	if (nick)
		*nick=query.value(0).toString();
	return query.value(1).toString();
}

void WordList::clear(const StorageKey& storage)
{
	if (!storage.isValid())
		return;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM words WHERE plugin=? AND storage=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.exec();
}

int WordList::count(const StorageKey& storage)
{
	if (!storage.isValid())
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT COUNT(*) FROM (SELECT name FROM words WHERE plugin=? AND storage=? GROUP BY NAME) AS ul");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.exec();
	if (!query.next())
		return 0;
	return query.value(0).toInt();
}

bool WordList::remove(const StorageKey& storage, const QString& name)
{
	if (!storage.isValid())
		return false;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM words WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.exec();
	return query.numRowsAffected();
}

QStringList WordList::getNames(const StorageKey& storage)
{
	if (!storage.isValid())
		return QStringList();
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("select name from words where plugin=? and storage=? group by name ORDER BY name");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	if (!query.exec())
		return QStringList();
	QStringList res;
	while (query.next())
		res.append(query.value(0).toString());
	return res;
}
