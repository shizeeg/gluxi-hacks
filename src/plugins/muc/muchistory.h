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
#ifndef MUCHISTORY_H_
#define MUCHISTORY_H_

#include "actiontype.h"

#include <QDateTime>

class Nick;
class QString;

class MucHistory
{
public:
	struct HistoryItem
	{
		///todo This should be temporary Nick *
		QDateTime dateTime;
		QString nick;
		QString message;
	};
public:
	MucHistory(int conferenceId);
	virtual ~MucHistory();

	void log(Nick *nick, Nick *dstNick, ActionType type, const QString& msg, bool priv,
			const QString& params = QString::null, const QDateTime& dateTime = QDateTime());

	QList<HistoryItem> missingHighlights(Nick *nick, QDateTime startDateTime) const;
private:
	int conferenceId_;
};

#endif /* MUCHISTORY_H_ */
