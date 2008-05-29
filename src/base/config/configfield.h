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
#ifndef CONFIGFIELD_H_
#define CONFIGFIELD_H_

#include <QString>

class ConfigField
{
public:
	enum FieldType {
		FIELDTYPE_UNKNOWN = 0,
		FIELDTYPE_TEXT = 1,
		FIELDTYPE_CHECKBOX = 2
	};

	ConfigField(FieldType type, const QString& name, const QString& description=QString(), const QString& value=QString());
	ConfigField(const QString& name, const QString& value=QString());
	ConfigField(const ConfigField& other);
	virtual ~ConfigField();
	
	FieldType type() const { return type_; }
	QString name() const { return name_; }
	QString description() const { return description_; }
	QString value() const { return value_; }
	
	void setType(FieldType type) { type_=type; }
	void setName(const QString& name) { name_=name; }
	void setDescription(const QString& description) { description_=description; }
	void setValue(const QString& value) { value_=value; }
private:
	FieldType type_;
	QString name_;
	QString description_;
	QString value_;
};

#endif /*CONFIGFIELD_H_*/
