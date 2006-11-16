#ifndef ALIST_H
#define ALIST_H

#include <QStringList>

#define ALIST_KICK 1
#define ALIST_VISITOR 2
#define ALIST_MODERATOR 3

class Conference;
class QDateTime;

class AList: public QStringList
{
public:
	AList(Conference *conf, int type);
	bool removeExpired();
// 	QString at(int idx);
	void removeAt(int idx);
	int removeAll(const QString& s);
	void append(const QString&s);
	void append(const QString&s, const QDateTime& expire);
	void removeItems();
	QString toString();
private:
	Conference *myParent;
	int myType;
	void load();
};

#endif
