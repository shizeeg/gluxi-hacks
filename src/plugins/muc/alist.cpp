#include "alist.h"
#include "alistitem.h"
#include "conference.h"
#include "base/common.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>
#include <QSqlError>
#include <QStack>

const static int CHILD_ALIST=9999;

AList::AList(Conference* conf, const QString& name, int type) :
	QList<AListItem*>()
{
	myParent=conf;
	name_=name;
	myType=type;
	convertUnknown();

	if (!removeExpired())
		load();
}

void AList::clear()
{
	if (isEmpty())
		return;
	AListItem* item;
	while (!isEmpty() && (item=QList<AListItem*>::takeFirst()))
		delete item;
}

int AList::count()
{
	return QList<AListItem*>::count();
}

int AList::indexOf(const AListItem& other)
{
	int idx=0;
	for (Iterator it=begin(); it!=end(); ++it)
	{
		AListItem* item=(*it);
		if ((*item)==other)
			return idx;
		++idx;
	}
	return -1;
}

int AList::indexOfSameCondition(const AListItem& other)
{
	int idx=0;
	for (Iterator it=begin(); it!=end(); ++it)
	{
		AListItem* item=(*it);
		if (item->isSameCondition(other))
			return idx;
		++idx;
	}
	return -1;
}

AListItem* AList::at(int idx)
{
	return QList<AListItem*>::at(idx);
}

void AList::load()
{
	QList<AListItem*>::clear();
	QSqlQuery query= DataStorage::instance()
		->prepareQuery("SELECT id, matcher, test, inv, value,"
		" reason, expire, child_id FROM conference_alists"
		" WHERE conference_id=? AND list=?");

	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.exec();
	while (query.next())
		QList<AListItem*>::append(itemFromQuery(query));
}

void AList::removeAt(int idx)
{
	if (idx>count()-1)
		return;
	AListItem* item=QList<AListItem*>::takeAt(idx);
	removeItem(item);
}

void AList::removeItem(AListItem* item)
{
	AListItem* it=item;
	while (it)
	{
		QSqlQuery query=DataStorage::instance()
			->prepareQuery("DELETE FROM conference_alists"
			" WHERE id=?");
		query.addBindValue(it->id());
		query.exec();
		it=it->child();
	}
	delete item;
}

bool AList::removeExpired()
{
	int numRemoved=0;
	QDateTime cur=QDateTime::currentDateTime();
	for (AList::iterator it=begin(); it!=end(); ++it)
	{
		AListItem* item=*it;
		if (item->expire().isValid() && (item->expire() < cur))
		{
			removeItem(item);
			++numRemoved;
		}
	}
	if (numRemoved)
	{
		QList<AListItem*>::clear();
		load();
	}
	return (numRemoved>0);
}

void AList::append(AListItem& item)
{
	if (item.child())
	{
		//Save child item first
		QStack<AListItem*> stack;
		AListItem* child=item.child();
		while (child)
		{
			stack.push(child);
			child=child->child();
		}
		while (!stack.isEmpty())
		{
			child=stack.pop();
			appendPrivate(*child, CHILD_ALIST);
		}
	}
	appendPrivate(item, myType);
	//TODO: Use some more optimezed way to update List
	load();
}

void AList::removeItems()
{
	for (Iterator it=begin(); it!=end(); ++it)
	{
		AListItem* item=(*it);
		removeItem(item);
	}
	QList<AListItem*>::clear();
}

QString AList::toString()
{
	QStringList res;
	int idx=1;
	for (Iterator it=begin(); it!=end(); ++it)
	{
		AListItem* item=(*it);
		QString line=QString("%1) %2").arg(idx++).arg(item->toString());
		res.append(line);
	}
	return res.join("\n");
}

AListItem* AList::itemFromQuery(QSqlQuery& query)
{
	AListItem* item=new AListItem();
	item->setId(query.value(0).toInt());
	item->setMatcherType((AListItem::MatcherType)(query.value(1).toInt()));
	item->setTestType((AListItem::TestType)(query.value(2).toInt()));
	item->setInvert(query.value(3).toBool());
	item->setValue(query.value(4).toString());
	item->setReason(query.value(5).toString());
	item->setExpire(query.value(6).toDateTime());

	int childId=query.value(7).toInt();
	if (childId>0)
	{
		QSqlQuery childQuery=DataStorage::instance()
				->prepareQuery("SELECT id, matcher, test, inv, value,"
				" reason, expire, child_id FROM conference_alists"
				" WHERE id=?");
		childQuery.addBindValue(childId);
		if (childQuery.exec() && childQuery.next())
			item->setChild(itemFromQuery(childQuery));
	}
	return item;
}

void AList::appendPrivate(AListItem& item, int type)
{
	QSqlQuery query= DataStorage::instance()
		->prepareQuery("INSERT INTO conference_alists"
		" (conference_id, list, matcher, test, inv, value, reason, expire, child_id)"
		" VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
	query.addBindValue(myParent->id());
	query.addBindValue(type);
	query.addBindValue(item.matcherType());
	query.addBindValue(item.testType());
	query.addBindValue(item.isInvert());
	query.addBindValue(item.value().toLower());
	query.addBindValue(item.reason().isEmpty() ? QVariant(QVariant::String) : item.reason());
	query.addBindValue(item.expire().isValid() ? item.expire() : QVariant(QVariant::DateTime));
	query.addBindValue(item.child() ? item.child()->id() : 0);
	if (!query.exec())
	{
		qDebug() << "Unable to append alist item: " << query.lastError().text();
		return;
	}
	item.setId(query.lastInsertId().toInt());
	if (item.id()<=0)
	{
		// Let's try PostgreSQL way to get ID
		query=DataStorage::instance()->prepareQuery("select currval('conference_alists_id_seq')");
		if (query.exec() && query.next())
			item.setId(query.value(0).toInt());
		else
		{
			qDebug() << "Unable to append alist item: " << query.lastError().text();
		}

		if (item.id()<=0)
			qDebug() << "Unable to get item id. Something happens";
	}
}

void AList::convertUnknown()
{
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("SELECT value, id FROM conference_alists"
		" WHERE conference_id=? AND list=? AND matcher=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(0);

	if (!query.exec())
		return;

	while (query.next())
	{
		QString origValue=query.value(0).toString();
		int id=query.value(1).toInt();
		QString value=origValue.toUpper();
		AListItem::MatcherType matcherType=AListItem::MatcherJid;
		AListItem::TestType testType=AListItem::TestExact;
		if (value.startsWith("JID "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::MatcherJid;
		}
		else if (value.startsWith("NICK "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::MatcherNick;
		}
		else if (value.startsWith("BODY "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::MatcherBody;
		}
		if (value.startsWith("EXP "))
		{
			value=value.section(' ', 1);
			testType=AListItem::TestRegExp;
		}
		QSqlQuery fixQuery = DataStorage::instance()
		->
		prepareQuery("UPDATE conference_alists SET"
			" matcher=?, test=?, value=?"
			" WHERE conference_id=? AND list=? AND id=?");
		fixQuery.addBindValue(matcherType);
		fixQuery.addBindValue(testType);
		fixQuery.addBindValue(value.toLower());

		fixQuery.addBindValue(myParent->id());
		fixQuery.addBindValue(myType);
		fixQuery.addBindValue(id);
		if (!fixQuery.exec())
			qDebug() << fixQuery.lastError().text();
		qDebug() << "Converted alist item: " << id << origValue;
	}
}
