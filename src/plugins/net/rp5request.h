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
#ifndef RP5REQUEST_H_
#define RP5REQUEST_H_
#include "base/asyncrequest.h"

class QHttp;
class MessageParser;
class QDomElement;

class Rp5Request: public AsyncRequest
{
	Q_OBJECT
public:
	Rp5Request(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser);
	virtual ~Rp5Request();
	virtual void exec();
private:
	QString myDest;
	QString myCmd;
	QHttp *http;
	QString parseXml(QDomElement e);
	void showForecast(QByteArray &data);
	void showForecastEx(QByteArray &data);

private slots:
	void httpRequestFinished(int, bool err);
};
#endif /* RSSREQUEST_H_ */
