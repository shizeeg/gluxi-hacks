#ifndef ALIAS_LIST
#define ALIAS_LIST

#include "base/config/storagekey.h"

#include <QList>
#include <QMap>

class QString;

class AliasList
{
public:
	AliasList();
	// 0 - err, 1 - added, 2 - updated
	int append(const StorageKey& storage, const QString& name, const QString& value);
	QMap<QString,QString> getAll(const StorageKey& storage);
	QString get(const StorageKey& storage, const QString&name);
	void clear(const StorageKey& storage);
	int count(const StorageKey& storage);
	bool remove(const StorageKey& storage, const QString& name);
};

#endif
