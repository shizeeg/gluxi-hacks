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
#ifndef ALIAS_H_
#define ALIAS_H_

#include <QString>

class Alias
{
public:
	Alias(const QString& name=QString(), const QString& value=QString());
	Alias(const Alias& other);
	virtual ~Alias();
	
	QString name() const { return name_; }
	QString value()  const { return value_; }
	bool isGlobal() const { return global_; }
	bool isEmpty() const { return name_.isEmpty(); }
	
	void setName(const QString& name) { name_=name; }
	void setValue(const QString& value) { value_=value; }
	void setGlobal(bool global) { global_=global; }
private:
	QString name_;
	QString value_;
	bool global_;
};

#endif /*ALIAS_H_*/
