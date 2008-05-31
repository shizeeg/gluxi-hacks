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
#ifndef IDENTITYITEM_H_
#define IDENTITYITEM_H_

#include "infoitem.h"

#include <QString>

class IdentityItem: public InfoItem
{
public:
	IdentityItem(const QString& category=QString(), const QString& type=QString(), const QString& name=QString());
	virtual ~IdentityItem();
	virtual gloox::Tag* infoTag();
private:
	QString category_;
	QString type_;
	QString name_;
};

#endif /*IDENTITYITEM_H_*/
