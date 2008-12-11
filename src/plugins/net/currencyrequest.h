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
#ifndef CURRENCYREQUEST_H
#define CURRENCYREQUEST_H

#include "base/asyncrequest.h"

#include <QString>

class QHttp;

class CurrencyRequest: public AsyncRequest
{
	Q_OBJECT
public:
	CurrencyRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest);
	~CurrencyRequest();
	void exec();
private:
	QString myDest;
	QHttp *http;
	int nres;
private slots:
	void httpRequestFinished(int, bool err);
	void httpListRequestFinished(int, bool err);
};
#endif
