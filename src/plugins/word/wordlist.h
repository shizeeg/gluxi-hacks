#ifndef WORDLIST_H
#define WORDLIST_H

#include "base/config/storagekey.h"

#include <QList>
#include <QMap>
#include <QStringList>

class WordList
{
public:
	WordList();
	// 0 - err, 1 - added, 2 - updated
	int append(const StorageKey& storage, const QString& name, const QString&nick, const QString& value);
	QMap<QString,QString> getAll(const StorageKey& storage, const QString&name);
	QString get(const StorageKey& storage, const QString&name, QString*nick = 0);
	void clear(const StorageKey& storage);
	int count(const StorageKey& storage);
	bool remove(const StorageKey& storage, const QString& name);
	QStringList getNames(const StorageKey& storage);
};

#endif
