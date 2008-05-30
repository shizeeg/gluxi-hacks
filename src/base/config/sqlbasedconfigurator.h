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
#ifndef SQLBASEDCONFIGURATOR_H_
#define SQLBASEDCONFIGURATOR_H_

#include "abstractconfigurator.h"
#include "storagekey.h"

class SqlBasedConfigurator: public AbstractConfigurator
{
public:
	SqlBasedConfigurator(const QString& targetJid, const StorageKey& key);
	virtual ~SqlBasedConfigurator();
	virtual QList<ConfigField> loadFields() = 0;
	virtual void saveFields(QList<ConfigField> fields) = 0;
protected:
	StorageKey key_;
	QList<ConfigField> loadAvailableFields();
	ConfigField loadValue(const ConfigField& field);
	void saveValue(const ConfigField& field);
};

#endif /*SQLBASEDCONFIGURATOR_H_*/
