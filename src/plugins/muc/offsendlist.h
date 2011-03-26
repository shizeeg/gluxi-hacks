#ifndef OFFSENDLIST_H
#define OFFSENDLIST_H

#include "base/config/storagekey.h"
#include "nick.h"
#include "jid.h"

#include <QList>
#include <QMap>
#include <QStringList>

class OffsendList
{
public:
	OffsendList();
	// 0 - err, 1 - added, 2 - updated
	int append(const StorageKey& storage, const QString& name, const QString& nick, const QString& value);
	QMap<QString,QString> getAll(const StorageKey& storage, const QString&name);
	QString get(const StorageKey& storage, const Nick* nick, int count = 1);
	QString at(const StorageKey& storage, const Nick* nick, int ndx);
	QString list(const StorageKey& storage, const Nick* nick);
	void clear(const StorageKey& storage, const Nick* nick);
	int count(const StorageKey& storage);
	int ndxToId(const StorageKey& storage, const Nick* nick, int index);
	bool remove(const StorageKey& storage, const Nick* nick, int id);
	QStringList getJids(const StorageKey& storage);
	QString jidToString(const StorageKey& storage, int jid);
	int stringToJid(const StorageKey& storage, const QString& jid);
	bool hasMessage(const QString& jid);
private:
	QStringList _jids;
};

#endif
