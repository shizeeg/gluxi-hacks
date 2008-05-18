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
	->prepareQuery("SELECT id, matcher, test, inv, value,"
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
		" (conference_id, list, matcher, test, inv, value, reason, expire)"
		" VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(item.matcherType());
	query.addBindValue(item.testType());
	query.addBindValue(item.isInvert());
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
		
		flags+=item->isInvert() ? "!" : " ";
		
		switch (item->matcherType())
		{
		case AListItem::MatcherUnknown:
			flags+="?";
			break;
		case AListItem::MatcherNick:
			flags+="N";
			break;
		case AListItem::MatcherJid:
			flags+="J";
			break;
		case AListItem::MatcherBody:
			flags+="B";
			break;
		case AListItem::MatcherResource:
			flags+="R";
			break;
		};
		
		switch (item->testType())
		{
			case AListItem::TestUnknown:
				flags+="?";
				break;
			case AListItem::TestExact:
				flags+=" ";
				break;
			case AListItem::TestRegExp:
				flags+="E";
				break;
			case AListItem::TestSubstring:
				flags+="S";
				break;
		}
		

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
	item->setTestType((AListItem::TestType)(query.value(2).toInt()));
	item->setInvert(query.value(3).toBool());
	item->setValue(query.value(4).toString());
	item->setReason(query.value(5).toString());
	item->setExpire(query.value(6).toDateTime());
	return item;
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
