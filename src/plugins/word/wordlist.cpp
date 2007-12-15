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

int WordList::append(const QList<int>& storage, const QString& name, const QString& nick, const QString& value)
{
	if (storage.count()!=2)
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("INSERT INTO words(plugin, storage, name, date, nick, value) VALUES (?, ?, ?, ?, ?, ?)");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
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
		query.addBindValue(storage[0]);
		query.addBindValue(storage[1]);
		query.addBindValue(name);
		query.addBindValue(nick);
		if (!query.exec())
			return 0;	
		return 2;
	}
	query.clear();
	query.prepare("SELECT COUNT(name) FROM words WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
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
		query.addBindValue(storage[0]);
		query.addBindValue(storage[1]);
		query.addBindValue(name);
		query.addBindValue(toDel);
		query.exec();
	}
	return 1;
}

QMap<QString,QString> WordList::getAll(const QList<int>& storage, const QString& name)
{
	QMap<QString,QString> map;
	if (storage.count()!=2)
		return map;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT nick, value FROM words WHERE plugin=? AND storage=? AND name=? ORDER BY date DESC");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	if (!query.exec())
		return map;
	while (query.next())
		map[query.value(0).toString()]=query.value(1).toString();
	return map;
}

QString WordList::get(const QList<int>& storage, const QString&name, QString* nick)
{
	if (storage.count()!=2)
		return QString::null;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT nick, value FROM words WHERE plugin=? AND storage=? AND name=? ORDER BY date DESC LIMIT 1");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	query.exec();
	if (!query.next())
		return QString::null;
	if (nick)
		*nick=query.value(0).toString();
	return query.value(1).toString();
}

void WordList::clear(const QList<int>& storage)
{
	if (storage.count()!=2)
		return;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM words WHERE plugin=? AND storage=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.exec();
}

int WordList::count(const QList<int>& storage)
{
	if (storage.count()!=2)
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT COUNT(DISTINCTROW name) FROM words WHERE plugin=? AND storage=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.exec();
	if (!query.next())
		return 0;
	return query.value(0).toInt();
}

bool WordList::remove(const QList<int>& storage, const QString& name)
{
	if (storage.count()!=2)
		return false;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM words WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	query.exec();
	return query.numRowsAffected();
}

