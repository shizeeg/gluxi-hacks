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
#ifndef MUCCONFIGURATOR_H_
#define MUCCONFIGURATOR_H_

#include "base/config/sqlbasedconfigurator.h"

class MucConfigurator: public SqlBasedConfigurator
{
public:
	MucConfigurator(const QString& targetJid, StorageKey key);
	virtual ~MucConfigurator();
	virtual void saveFields(QList<ConfigField> fields);

	bool isApplyAlistsToMembers() const { return applyAlistsToMembers_; }
	bool isCheckAlistsEveryPresence() const { return checkAlistsEveryPresence_; }
	bool isDevoiceNoVCard() const { return devoiceNoVCard_; }
	QString devoiceNoVCardReason() const { return devoiceNoVCardReason_; }
	bool isQueryVersionOnJoin() const { return queryVersionOnJoin_; }
	int queryVersionTimeout() const { return queryVersionTimeout_; }
private:
	bool applyAlistsToMembers_;
	bool checkAlistsEveryPresence_;
	bool devoiceNoVCard_;
	QString devoiceNoVCardReason_;
	bool queryVersionOnJoin_;
	int queryVersionTimeout_;
	void parse();

};

#endif /*MUCCONFIGURATOR_H_*/
