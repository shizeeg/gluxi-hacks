#include "aliaslist.h"
#include "base/datastorage.h"

#include <QList>
#include <QVariant>
#include <QSqlQuery>

AliasList::AliasList()
{
}

int AliasList::append(const QList<int>& storage, const QString& name, const QString& value)
{
	if (storage.count()!=2)
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("INSERT INTO aliases(plugin, storage, name, value) VALUES (?, ?, ?, ?)");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	query.addBindValue(value);
	if (query.exec())
		return 1;
	query.clear();
	query.prepare("UPDATE aliases SET value=? WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(value);
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	if (query.exec())
		return 2;
	return 0;
}

QMap<QString,QString> AliasList::getAll(const QList<int>& storage)
{
	QMap<QString,QString> map;
	if (storage.count()!=2)
		return map;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT name, value FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.exec();
	while (query.next())
		map[query.value(0).toString()]=query.value(1).toString();
	return map;
}

QString AliasList::get(const QList<int>& storage, const QString&name)
{
	if (storage.count()!=2)
		return QString::null;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT value FROM aliases WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	query.exec();
	if (!query.next())
		return QString::null;
	return query.value(0).toString();
}

void AliasList::clear(const QList<int>& storage)
{
	//WTF???
	if (storage.count()!=2)
		return;
	
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.exec();
}

int AliasList::count(const QList<int>& storage)
{
	if (storage.count()!=2)
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT COUNT(name) FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.exec();
	if (!query.next())
		return 0;
	return query.value(0).toInt();
}

bool AliasList::remove(const QList<int>& storage, const QString& name)
{
	if (storage.count()!=2)
		return false;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM aliases WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage[0]);
	query.addBindValue(storage[1]);
	query.addBindValue(name);
	query.exec();
	return query.numRowsAffected();
}
