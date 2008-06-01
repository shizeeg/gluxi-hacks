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
#include "sqlbasedconfigurator.h"
#include "base/datastorage.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QSet>
#include <QtDebug>

SqlBasedConfigurator::SqlBasedConfigurator(const QString& targetJid, const StorageKey& key)
	: AbstractConfigurator(targetJid)
{
	key_=key;
}

SqlBasedConfigurator::~SqlBasedConfigurator()
{
}

QList<ConfigField> SqlBasedConfigurator::loadFields()
{
	QList<ConfigField> availableFieldList=loadAvailableFields();
	QList<ConfigField> fieldList;
	for (QList<ConfigField>::iterator it=availableFieldList.begin(); it!=availableFieldList.end(); ++it)
	{
		fieldList << loadValue(*it);
	}
	return fieldList;
}

void SqlBasedConfigurator::saveFields(QList<ConfigField> fields)
{
	QList<ConfigField> availableFieldList=loadAvailableFields();
	QSet<QString> fieldNames;
	for (QList<ConfigField>::iterator it=availableFieldList.begin(); it!=availableFieldList.end(); ++it)
		fieldNames.insert((*it).name());
	
	for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
		if (fieldNames.contains((*it).name()))
			saveValue(*it);
}

QList<ConfigField> SqlBasedConfigurator::loadAvailableFields()
{
	QSqlQuery query=DataStorage::instance()
		->prepareQuery("SELECT name, field_type, description, default_value FROM configuration_fields"
				" WHERE plugin=? ORDER BY priority");
	qDebug() << "plugin: " << key_.plugin();
	query.addBindValue(key_.plugin());
	QList<ConfigField> fieldList;
	if (!query.exec())
	{
		qDebug() << query.lastError().text();
		return fieldList;
	}
	while (query.next())
	{
		QString name=query.value(0).toString();
		ConfigField::FieldType fieldType=(ConfigField::FieldType)(query.value(1).toInt());
		QString description=query.value(2).toString();
		QString defaultValue=query.value(3).toString();
		fieldList << ConfigField(fieldType,name, description, defaultValue);
	}
	return fieldList;
}

ConfigField SqlBasedConfigurator::loadValue(const ConfigField& field)
{
	QSqlQuery query;
	
	query=DataStorage::instance()
		->prepareQuery("SELECT value FROM configuration WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(key_.plugin());
	query.addBindValue(key_.storage());
	query.addBindValue(field.name());
	
	if (query.exec() && query.next())
	{
		ConfigField newField(field);
		newField.setValue(query.value(0).toString());
		return newField;
	}
	else
		return field;
}

void SqlBasedConfigurator::saveValue(const ConfigField& field)
{
	QSqlQuery query;
	
	query=DataStorage::instance()
		->prepareQuery("SELECT value FROM configuration WHERE plugin=? AND storage=? AND name=?");
	query.addBindValue(key_.plugin());
	query.addBindValue(key_.storage());
	query.addBindValue(field.name());
	
	if (query.exec() && query.next())
	{
		if (query.value(0).toString()!=field.value())
		{
			query=DataStorage::instance()
				->prepareQuery("UPDATE configuration SET value=? WHERE plugin=? AND storage=? AND name=?");
			query.addBindValue(field.value());
			query.addBindValue(key_.plugin());
			query.addBindValue(key_.storage());
			query.addBindValue(field.name());
			query.exec();
		}
	}
	else
	{
		query=DataStorage::instance()
			->prepareQuery("INSERT INTO configuration (plugin, storage, name, value) VALUES (?, ?, ?, ?)");
		query.addBindValue(key_.plugin());
		query.addBindValue(key_.storage());
		query.addBindValue(field.name());
		query.addBindValue(field.value());
		query.exec();
	}
}
