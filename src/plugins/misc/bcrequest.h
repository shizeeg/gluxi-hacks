/***************************************************************************
 *   Copyright (C) 2008 by Sidgyck                                         *
 *   sidgyck@gmail.com                                                     *
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
#ifndef BCREQEST_H
#define BCREQEST_H

#define FAC "define fac (x) { if (x <= 1) return (1); return (fac(x-1) * x); };"

#include "base/asyncrequest.h"

#include <QProcess>

class MessageParser;

class BcRequest: public AsyncRequest
{
	Q_OBJECT
public:
	BcRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser &parser);
	virtual ~BcRequest();
	virtual void exec();
private:
	QProcess* proc_;
	QString myDest;
private slots:
	void onProcessFinished();
	void onStateChanged(QProcess::ProcessState newState);
};

#endif /* BCREQUEST_H */
