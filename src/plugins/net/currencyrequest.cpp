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

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QtDebug>

CurrencyRequest::CurrencyRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	http=0;
}

CurrencyRequest::~CurrencyRequest()
{
	if (http)
		delete http;
	http=0;
}

void CurrencyRequest::exec()
{
	if ( myDest.endsWith("list") )
	{
		QString url=QString("http://www.xe.com/ucc/full/");
		QUrl qurl(url);
		http=new QHttp(qurl.host());
		connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpListRequestFinished(int, bool)));
		http->get(qurl.toEncoded());
		return;
	}

	QString amount   = myDest.section(' ', 0, 0);
	QString from 	 = myDest.section(' ', 1, 1);
	QString to	 = myDest.section(' ', 2, 2);

	if( from.toUpper() == "RUB" && to.isEmpty() ) {
		to = "USD";
	} else if( to.isEmpty() ) {
		to = "RUB";
	} 
	
	if (myDest.isEmpty() || amount.isEmpty() || from.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: net currency <amount> <from> <to>");
		deleteLater();
		return;
	}

	QString url=QString("http://www.xe.com/ucc/convert.cgi?Amount=%1&From=%2&To=%3").arg(amount).arg(from.toUpper()).arg(to.toUpper());
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

	QString from = removeHtml(getValue(buf,"align=\"right\" class=\"XEenlarge\"><h2 class=\"XE\" style=\"color:#333\">(.*)<!--")).trimmed();
	QString to   = removeHtml(getValue(buf,"align=\"left\" class=\"XEenlarge\"><h2 class=\"XE\" style=\"color:#333\">(.*)<!--")).trimmed();
	
	QString curr1 = myDest.section(' ', 1, 1);
	QString curr2 = myDest.section(' ', 2, 2);
	QString msg   = "unknown currency: %1";

	if( from.isEmpty() || to.isEmpty() ) {
		if( !curr2.isEmpty() && from == to) {
			 msg.append(" or ").append(curr2);
		}
		plugin()->reply( stanza(), msg.arg(curr1));
	} else {
		plugin()->reply( stanza(), QString("%1 = %2").arg(from).arg(to).simplified().replace(",", " "));
	}

	deleteLater();
}

void CurrencyRequest::httpListRequestFinished(int, bool err)
{
	if (err || http->lastResponse().statusCode()!=200) {
		plugin()->reply(stanza(),"Failed to fetch list from XE.com: "+http->lastResponse().reasonPhrase());
		deleteLater();
		return;
	}

	QString buf=http->readAll();
	QRegExp exp("<select(.*)/select>");
	exp.setMinimal(TRUE);
	QString currencies, data;
	int ps=0;

	while (nres>0 && ((ps=exp.indexIn(buf,ps))>=0))	{
		QStringList lst=exp.capturedTexts();
		ps+=exp.matchedLength();
		currencies=lst[0];
	}

	QStringList clist = currencies.split("\n");

	for (int i = 0; i < clist.size(); ++i) {
		data += (removeHtml(getValue(clist[i], ">(.*)</option>")).trimmed() + "\n");
	}

	plugin()->reply(stanza(), "Available currencies:\n" + data.trimmed());
	deleteLater();
}

