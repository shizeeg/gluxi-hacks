/***************************************************************************
 *   Copyright (C) 2009 by Sidgyck                                         *
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
#include "rssrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "base/messageparser.h"

#include <QtXmlPatterns>
#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QProcess>
#include <QtDebug>
#include <QMutexLocker>
#include <QTextCodec>

RssRequest::RssRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest	= parser.nextToken();
	myQuery = parser.joinBody();
	http=0;
	proc=0;
}

RssRequest::~RssRequest()
{
	if (http)
		delete http;
	http=0;
	if (proc)
		delete proc;
	proc=0;
}

void RssRequest::exec()
{
	if (myDest.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: !net rss <feed> [query]");
		deleteLater();
		return;
	}

	QString url = myDest;

	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.toEncoded());
}

void RssRequest::httpRequestFinished(int, bool err)
{
	QHttpResponseHeader lastResponse = http->lastResponse();
	if (err || lastResponse.statusCode()!=200 ) {
		if ( lastResponse.statusCode()==301 || lastResponse.isValid() ) {
			http->close();
			QUrl qurl(lastResponse.value("location"));
			http->setHost(qurl.host());
			http->get(qurl.toEncoded());
		} else {
			plugin()->reply(stanza(), QString("Failed to fetch RSS from %1: ")
				.arg(myDest) +http->lastResponse().reasonPhrase());
			deleteLater();
		}
		return;
	}

	items.clear();
	QXmlStreamReader xml; 
	QString currentTag;
	QMap<QString, QString> items;
	xml.addData(http->readAll());
	QString tag;
	for (int i = 0; !xml.atEnd(); xml.readNext())
	{
		if (xml.isStartElement())
		{
			if (xml.name() == "item")
			{
				i++;
			} 
			else if (xml.name() == "description" ||
				 xml.name() == "title" || xml.name() == "link")
			{
				items.insert(
					xml.name().toString()
					+= QString::number(i),
					xml.readElementText());
			}
		}
	}

	if( items.size() <= 0 ) {
		plugin()->reply(stanza(), "no rss feed found.");
		deleteLater();
		return;
	}
	
	if (proc) delete proc;
	proc = new QProcess();
	QStringList args;
	args << "-dump" << "-nolist" << "-display_charset=utf-8" << "-assume_charset=utf-8" << "-stdin" << "-force_html" << "-width=32768";
	proc->start("lynx", args);

	if (!proc->waitForStarted())
	{
		plugin()->reply(stanza(),"Unable to launch lynx -dump");
		return;
	}
	int num = myQuery.toInt();
	if ( num >= items.size() ) num = items.size() - 1;
	if ( num <= 0 ) num = 1;

	proc->write(items[QString("description") + QString().setNum(num)].toUtf8());
	proc->closeWriteChannel();

	if (!proc->waitForFinished())
	{
		plugin()->reply(stanza(),"Error: lynx timeout");
		return;
	}
	QString res = QString("%1 (%2)\n%3")
		.arg(items[QString("title") + QString().setNum(num)])
		.arg(items[QString("link")  + QString().setNum(num)]).arg(QString(proc->readAll()));
	
	plugin()->reply(stanza(), res.trimmed());
	/*	int num = myQuery.toInt();
	if ( num <= 0 ) num = 1;
	if ( num >= items.size() ) num = items.size() - 1;
	plugin()->reply(stanza(), items.at(num));
	*/
	deleteLater();
}

