/***************************************************************************
 *   Copyright (C) 2008 by Dmitry Nezhevenko                               *
 *   dion@inhex.net                                                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "acllist.h"
#include "base/datastorage.h"

#include <QList>
#include <QVariant>
#include <QSqlQuery>

AclList::AclList()
{
}

int AclList::append(const QString& name, const QString& value)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("INSERT INTO acl(name, value) VALUES (?, ?)");
	query.addBindValue(name);
	query.addBindValue(value);
	if (query.exec())
		return 1;
	query.clear();
	query.prepare("UPDATE acl SET value=? WHERE name=?");
	query.addBindValue(value);
	query.addBindValue(name);
	if (query.exec())
		return 2;
	return 0;
}

QString AclList::get(const QString& name)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT value FROM acl WHERE name=?");
	query.addBindValue(name);
	query.exec();
	if (!query.next())
		return QString::null;
	return query.value(0).toString();
}

QMap<QString, QString> AclList::getAll()
{
	QMap<QString, QString> res;
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT name, value FROM acl");
	query.exec();
	while (query.next())
		res.insert(query.value(0).toString(), query.value(1).toString());
	return res;
}

void AclList::clear()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("TRUNCATE acl");
	query.exec();
}

int AclList::count()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT COUNT(name) FROM acl");
	query.exec();
	if (!query.next())
		return 0;
	return query.value(0).toInt();
}

bool AclList::remove(const QString& name)
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("DELETE FROM acl WHERE name=?");
	query.addBindValue(name);
	query.exec();
	return query.numRowsAffected();
}
