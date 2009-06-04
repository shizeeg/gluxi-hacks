/***************************************************************************
 *   Copyright (C) 2009 by Dmitry Nezhevenko                               *
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
#ifndef JIDSTAT_H_
#define JIDSTAT_H_

#include "actiontype.h"

#include <QString>
#include <QDateTime>

class JidStat
{
public:
	struct StatAction
	{
		ActionType type;
		QString reason;
		QString verName;
		QString verVersion;
		QString verOs;
	};

public:
	JidStat(int jidId);
	virtual ~JidStat();

	static JidStat *queryReadOnly(int jidId);
	static QString queryReport(int conferenceId, const QString& type, int numRes = 10);
	static QString availableReports();

	void commit();
	void setLastAction(ActionType type, const QString& reason);
	void setVersion(const QString& name, const QString& version, const QString& os);
	void updateOnlineTime();
	void statMessage(const QString& msg);
	void statReply();
	void statSubject(const QString& subject);


	StatAction lastAction() const;
private:
	int id_;
	int jidId_;
	bool readOnly_;
	QDateTime dateTime_;

	bool load();
	void create();
};

#endif /* JIDSTAT_H_ */
