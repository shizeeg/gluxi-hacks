#ifndef WORDLIST_H
#define WORDLIST_H

#include <QList>
#include <QMap>
#include <QStringList>

class WordList
{
public:
	WordList();
	// 0 - err, 1 - added, 2 - updated
	int append(const QList<int>& storage, const QString& name, const QString&nick, const QString& value);
	QMap<QString,QString> getAll(const QList<int>& storage, const QString&name);
	QString get(const QList<int>& storage, const QString&name, QString*nick = 0);
	void clear(const QList<int>& storage);
	int count(const QList<int>& storage);
	bool remove(const QList<int>& storage, const QString& name);
};

#endif
