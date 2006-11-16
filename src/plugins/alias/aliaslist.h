#ifndef ALIAS_LIST
#define ALIAS_LIST

#include <QList>
#include <QMap>

class QString;

class AliasList
{
public:
	AliasList();
	// 0 - err, 1 - added, 2 - updated
	int append(const QList<int>& storage, const QString& name, const QString& value);
	QMap<QString,QString> getAll(const QList<int>& storage);
	QString get(const QList<int>& storage, const QString&name);
	void clear(const QList<int>& storage);
	int count(const QList<int>& storage);
	bool remove(const QList<int>& storage, const QString& name);
};

#endif
