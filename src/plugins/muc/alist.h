#ifndef ALIST_H
#define ALIST_H

#include <QStringList>

#define ALIST_KICK 1
#define ALIST_VISITOR 2
#define ALIST_MODERATOR 3
#define ALIST_BAN 4
#define ALIST_CMD 5
#define ALIST_PARTICIPANT 6

class AListItem;

class Conference;
class QDateTime;
class QSqlQuery;

class AList: private QList<AListItem*>
{
public:
	AList(Conference *conf, const QString& name, int type);
	bool removeExpired();
// 	QString at(int idx);
	void removeAt(int idx);
	void append(AListItem& item);
	void removeItems();
	QString toString();
	void clear();
	int count();
	int indexOf(const AListItem& other);
	int indexOfSameCondition(const AListItem& other);
	AListItem* at(int idx);
	QString name() const { return name_; }
private:
	Conference *myParent;
	QString name_;
	int myType;
	void load();
	void convertUnknown();
	static AListItem* itemFromQuery(QSqlQuery& query);
	void appendPrivate(AListItem& item, int type);
	void removeItem(AListItem* item);
};

#endif
