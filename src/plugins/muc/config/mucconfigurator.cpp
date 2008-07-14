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
#include "mucconfigurator.h"

MucConfigurator::MucConfigurator(const QString& targetJid, StorageKey key)
	: SqlBasedConfigurator(targetJid, key)
{
	parse();
}

MucConfigurator::~MucConfigurator()
{
}

void MucConfigurator::saveFields(QList<ConfigField> fields)
{
	SqlBasedConfigurator::saveFields(fields);
	parse();
}

void MucConfigurator::parse()
{
	QList<ConfigField> fields=loadFields();
	for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
	{
		ConfigField field=*it;
		if (field.name()=="alists_members")
			applyAlistsToMembers_=field.boolValue();
		if (field.name()=="alists_every_presence")
			checkAlistsEveryPresence_=field.boolValue();
		if (field.name()=="devoice_no_vcard")
			devoiceNoVCard_=field.boolValue();
		if (field.name()=="devoice_no_vcard_reason")
			devoiceNoVCardReason_=field.value();
		if (field.name()=="query_version_on_join")
			queryVersionOnJoin_=field.boolValue();
		if (field.name()=="query_version_timeout")
			queryVersionTimeout_=field.value().toInt();
	}
}
