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
#include "currencyrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "base/messageparser.h"

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QtDebug>

CurrencyRequest::CurrencyRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	:AsyncRequest(-1, plugin, from, 300)
{
	http   = 0;
	myCmd  = parser.nextToken().toUpper();
	myFrom = parser.nextToken();
	myTo   = parser.nextToken();

	parser.back(2);
	myDest = parser.joinBody();
	bool ok;

	myCmd.toFloat(&ok);
	if (ok)
		myAmount = myCmd;
}

CurrencyRequest::~CurrencyRequest()
{
	if (http)
		delete http;
	http=0;
}

void CurrencyRequest::exec()
{
	if (myCmd=="LIST" || myCmd=="SEARCH")
	{
		QString url=QString("http://www.xe.com/ucc/full/");
		QUrl qurl(url);
		http=new QHttp(qurl.host());
		connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpListRequestFinished(int, bool)));
		http->get(qurl.toEncoded());
		return;
	}
	else if (myCmd.isEmpty() || myCmd=="HELP")
	{
		// show full help if requested
		help(myCmd.isEmpty());
		return;
	}

	if (myFrom.toUpper() == "RUB" && myTo.isEmpty())
		myTo = "USD";
	else if (myTo.isEmpty() && myFrom.length() == 3)
		myTo = "RUB";
	else if(myFrom.length() != 3 || myTo.length() != 3)
	{
		help(true);
		return;
	}

	QString url = QString(
		"http://www.xe.com/ucc/convert.cgi?Amount=%1&From=%2&To=%3")
		.arg(myAmount)
		.arg(myFrom.toUpper())
		.arg(myTo.toUpper());

	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.toEncoded());
}

void CurrencyRequest::httpRequestFinished(int, bool err)
{
	if (err || http->lastResponse().statusCode()!=200)
	{
		plugin()->reply(stanza(),"Failed to fetch from XE.com: "+http->lastResponse().reasonPhrase());
		deleteLater();
		return;
	}
	QString buf=http->readAll();
	QString from = getValue(buf, "align=\"right\" class=\"rate\" >(.*)<!--").trimmed();
	QString to   = getValue(buf, "align=\"left\" class=\"rate\" >(.*)<!--").trimmed();
	QString msg;

	if (from.isEmpty())
		msg += myFrom;
	if (to.isEmpty())
		msg += (msg.isEmpty()) ? myTo : " and/or " + myTo;
	
	if (msg.isEmpty())
	{
		plugin()->reply(stanza(), QString("%1 = %2")
				.arg(from)
				.arg(to).simplified().replace(',', ' '));
	}
	else
	{
		plugin()->reply(stanza(), msg.append(" is unknown currency!"));
	}

	deleteLater();
}

void CurrencyRequest::httpListRequestFinished(int, bool err)
{
	if (err || http->lastResponse().statusCode()!=200) 
	{
		plugin()->reply(stanza(), "Failed to fetch list from XE.com: " +
				http->lastResponse().reasonPhrase());
		deleteLater();
		return;
	}

	QString buf=http->readAll();
	QStringList clist = getValue(buf, "<select(.*)/select>")
		.trimmed().split("\n");
	QString res, tmp;
	
	for (int i = 0; i < clist.size(); i++)
	{
		tmp = (removeHtml(getValue(clist[i], ">(.*)</option>")).trimmed() + "\n");
		if (myCmd != "SEARCH")
		{
			res += tmp;
		}
		else
		{
			if (tmp.contains(myDest, Qt::CaseInsensitive))
				res += tmp;
		}
	}
	res = res.trimmed();
	if (res.isEmpty())
		plugin()->reply(stanza(), "Nothing found");
	else
		plugin()->reply(stanza(), "Available currencies:\n" + res);
	deleteLater();
}
/*
 * help(true)  - show oneline legacy help
 * help(false) - show full manual
 */
void CurrencyRequest::help(bool usage)
{
	QStringList help; help
	<< "!net currency <amount> <from> [to]"
	<< "amount         amount of money"
	<< "from           currency to convert from ('USD' for ex.)"
	<< "to             currency convert to (optional. 'RUB' by default)"
	<< "Options:"
	<< "!net currency <OPTION>"
	<< "help           this help screen"
	<< "list           list all available currencies"
	<< "search <text>  search for specified currency";
	if (usage)
		plugin()->reply(stanza(), help[0]);
	else 
		plugin()->reply(stanza(), "\nusage: " + help.join("\n"));
	deleteLater();
}
