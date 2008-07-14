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
#ifndef NICKASYNCREQUEST_H_
#define NICKASYNCREQUEST_H_

#include "base/asyncrequest.h"

class NickAsyncRequest: public AsyncRequest
{
	Q_OBJECT
public:
	NickAsyncRequest(int id, BasePlugin *plugin, gloox::Stanza *from,
			int timeout=600, bool notifyOnTimeout=false);
	virtual ~NickAsyncRequest();

	QString conference() const { return conference_ ; }
	QString jid() const { return jid_; }
	QString nick() const { return nick_; }

	void setConference(const QString& conference) { conference_=conference; }
	void setJid(const QString& jid) { jid_=jid; }
	void setNick(const QString& nick) { nick_=nick; }
private:
	QString conference_;
	QString jid_;
	QString nick_;
};

#endif /* NICKASYNCREQUEST_H_ */
