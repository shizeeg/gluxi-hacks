#include "aliaslist.h"
#include "base/datastorage.h"

#include <QList>
#include <QVariant>
#include <QSqlQuery>

AliasList::AliasList()
{
}

int AliasList::append(const StorageKey& storage, const Alias& alias)
{
	if (!storage.isValid())
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("INSERT INTO aliases(plugin, storage, global, name, value) VALUES (?, ?, ?, ?, ?)");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(alias.isGlobal());
	query.addBindValue(alias.name());
	query.addBindValue(alias.value());
	if (query.exec())
		return 1;
	query.clear();
	query.prepare("UPDATE aliases SET global=?, value=? WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(alias.isGlobal());
	query.addBindValue(alias.value());
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(alias.name());
	if (query.exec())
		return 2;
	return 0;
}

QMap<QString,Alias> AliasList::getAll(const StorageKey& storage)
{
	QMap<QString,Alias> map;
	if (!storage.isValid())
		return map;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT global, name, value FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.exec();
	while (query.next())
	{
		Alias alias(query.value(1).toString(),query.value(2).toString());
		alias.setGlobal(query.value(0).toBool());
		map[alias.name()]=alias;
	}
	return map;
}

Alias AliasList::get(const StorageKey& storage, const QString& name)
{
	if (!storage.isValid())
		return Alias();
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT global, value FROM aliases WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.exec();
	if (!query.next())
		return Alias();
	Alias alias(name, query.value(1).toString());
	alias.setGlobal(query.value(0).toBool());
	return alias;
}

void AliasList::clear(const StorageKey& storage)
{
	//WTF???
	if (!storage.isValid())
		return;
	
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.exec();
}

int AliasList::count(const StorageKey& storage)
{
	if (!storage.isValid())
		return 0;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT COUNT(name) FROM aliases WHERE plugin=? AND storage=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.exec();
	if (!query.next())
		return 0;
	return query.value(0).toInt();
}

bool AliasList::remove(const StorageKey& storage, const QString& name)
{
	if (!storage.isValid())
		return false;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM aliases WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(storage.plugin());
	query.addBindValue(storage.storage());
	query.addBindValue(name);
	query.exec();
	return query.numRowsAffected();
}
