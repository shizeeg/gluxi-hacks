#include "alist.h"
#include "alistitem.h"
#include "conference.h"
#include "base/common.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>

AList::AList(Conference* conf, int type) :
	QList<AListItem*>()
{
	myParent=conf;
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

AListItem* AList::at(int idx)
{
	return QList<AListItem*>::at(idx);
}

void AList::load()
{
	clear();
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("SELECT id, matcher, regexp, value,"
		" reason, expire FROM conference_alists"
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
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("DELETE FROM conference_alists"
		" WHERE id=? AND conference_id=? AND list=?");

	query.addBindValue(QList<AListItem*>::at(idx)->id());
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.exec();
	delete QList<AListItem*>::takeAt(idx);
}

bool AList::removeExpired()
{
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("DELETE FROM conference_alists"
		" WHERE conference_id=? AND list=? AND expire<=?");

	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(QDateTime::currentDateTime());
	query.exec();
	if (query.numRowsAffected())
	{
		load();
		return true;
	}
	return false;
}

void AList::append(const AListItem& item)
{
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("INSERT INTO conference_alists"
		" (conference_id, list, matcher, regexp, value, reason, expire)"
		" VALUES (?, ?, ?, ?, ?, ?, ?)");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(item.matcherType());
	query.addBindValue(item.isRegExp());
	query.addBindValue(item.value().toLower());
	query.addBindValue(item.reason().isEmpty() ? QVariant(QVariant::String) : item.reason());
	query.addBindValue(item.expire().isValid() ? item.expire() : QVariant(QVariant::DateTime));

	query.exec();
	//TODO: Use some more optimezed way to update List
	load();
}

void AList::removeItems()
{
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("DELETE FROM conference_alists"
		" WHERE conference_id=? AND list=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.exec();
	clear();
}

QString AList::toString()
{
	QStringList res;
	int idx=1;
	for (Iterator it=begin(); it!=end(); ++it)
	{
		QString flags;
		AListItem* item=(*it);
		switch (item->matcherType())
		{
		case AListItem::UNKNOWN:
			flags+="?";
			break;
		case AListItem::NICK:
			flags+="N";
			break;
		case AListItem::JID:
			flags+="J";
			break;
		case AListItem::BODY:
			flags+="B";
			break;
		}
		flags+=item->isRegExp() ? "E" : " ";

		QString line=QString("%1) %2 %3").arg(idx++).arg(flags).arg(item->value());
		if (item->expire().isValid())
		{
			int delta=QDateTime::currentDateTime().secsTo(item->expire());
			if (delta>0)
				line+=QString("	[%1]").arg(secsToString(delta));
			else
				line+=QString(" [EXPIRED]");
		}
		if (!item->reason().isEmpty())
			line+=" // "+item->reason();
		res.append(line);
	}
	return res.join("\n");
}

AListItem* AList::itemFromQuery(QSqlQuery& query)
{
	AListItem* item=new AListItem();
	item->setId(query.value(0).toInt());
	item->setMatcherType((AListItem::MatcherType)(query.value(1).toInt()));
	item->setIsRegExp(query.value(2).toBool());
	item->setValue(query.value(3).toString());
	item->setReason(query.value(4).toString());
	item->setExpire(query.value(5).toDateTime());
	return item;
}

void AList::convertUnknown()
{
	QSqlQuery query= DataStorage::instance()
	->prepareQuery("SELECT value FROM conference_alists"
		" WHERE conference_id=? AND list=? AND matcher=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(0);

	if (!query.exec())
		return;

	while (query.next())
	{
		QString origValue=query.value(0).toString();
		QString value=origValue.toUpper();
		AListItem::MatcherType matcherType=AListItem::JID;
		bool isRegExp=false;
		if (value.startsWith("JID "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::JID;
		}
		else if (value.startsWith("NICK "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::NICK;
		}
		else if (value.startsWith("BODY "))
		{
			value=value.section(' ', 1);
			matcherType=AListItem::BODY;
		}
		if (value.startsWith("EXP "))
		{
			value=value.section(' ', 1);
			isRegExp=true;
		}
		QSqlQuery fixQuery = DataStorage::instance()
		->
		prepareQuery("UPDATE conference_alists SET"
			" matcher=?, regexp=?, value=?"
			" WHERE conference_id=? AND list=? AND value=?");
		fixQuery.addBindValue(matcherType);
		fixQuery.addBindValue(isRegExp);
		fixQuery.addBindValue(value.toLower());

		fixQuery.addBindValue(myParent->id());
		fixQuery.addBindValue(myType);
		fixQuery.addBindValue(origValue);
		fixQuery.exec();
	}
}
