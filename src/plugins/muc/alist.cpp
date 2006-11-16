#include "alist.h"
#include "base/common.h"
#include "conference.h"

#include <QtDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QDateTime>

AList::AList(Conference* conf, int type)
	:QStringList()
{
	myParent=conf;
	myType=type;
	if (!removeExpired())
		load();
}

void AList::load()
{
	clear();
	QSqlQuery query;
	query.prepare("SELECT value FROM conference_alists WHERE conference_id=? AND list=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.exec();
	while (query.next())
		QStringList::append(query.value(0).toString());
}

void AList::removeAt(int idx)
{
	if (idx>count()-1)
		return;
	QSqlQuery query;
	query.prepare("DELETE FROM conference_alists WHERE conference_id=? AND list=? AND value=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(QStringList::at(idx));
	query.exec();
	QStringList::removeAt(idx);
}

int AList::removeAll(const QString& s)
{
	QSqlQuery query;
	query.prepare("DELETE FROM conference_alists WHERE conference_id=? AND list=? AND value=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.addBindValue(s);
	query.exec();
	return QStringList::removeAll(s);
}

bool AList::removeExpired()
{
	QSqlQuery query;
	query.prepare("DELETE FROM conference_alists WHERE conference_id=? AND list=? AND expire<=?");
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

void AList::append(const QString&s)
{
	QSqlQuery query;
	query.prepare("INSERT INTO conference_alists (conference_id, value, list) VALUES (?, ?, ?)");
	query.addBindValue(myParent->id());
	query.addBindValue(s);
	query.addBindValue(myType);
	query.exec();
	QStringList::append(s);
}

void AList::append(const QString&s, const QDateTime& expire)
{
	QSqlQuery query;
	query.prepare("INSERT INTO conference_alists (conference_id, value, list, expire) VALUES (?, ?, ?, ?)");
	query.addBindValue(myParent->id());
	query.addBindValue(s);
	query.addBindValue(myType);
	query.addBindValue(expire);
	query.exec();
	QStringList::append(s);
}
void AList::removeItems()
{
	QSqlQuery query;
	query.prepare("DELETE FROM conference_alists WHERE conference_id=? list=?");
	query.addBindValue(myType);
	query.exec();
	QStringList::clear();
}

QString AList::toString()
{
	QStringList res;
	QSqlQuery query;
	query.prepare("SELECT value, expire FROM conference_alists WHERE conference_id=? AND list=?");
	query.addBindValue(myParent->id());
	query.addBindValue(myType);
	query.exec();
	int idx=1;
	while (query.next())
	{
		QString line=QString("%1) %2").arg(idx++).arg(query.value(0).toString().toLower());
		if (!query.value(0).isNull())
		{
			int delta=QDateTime::currentDateTime().secsTo(query.value(1).toDateTime());
			if (delta>0)
				line+=QString("	(%1)").arg(secsToString(delta));
		}
		res.append(line);
	}
	return res.join("\n");
}
