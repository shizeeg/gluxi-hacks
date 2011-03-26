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
#include "weatherrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "base/messageparser.h"

//#include <QXml>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QProcess>
#include <QtDebug>
#include <QMutexLocker>
#include <QTextCodec>

WeatherRequest::WeatherRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest = parser.joinBody().trimmed();
	http   = 0;
}

WeatherRequest::~WeatherRequest()
{
	if (http)
		delete http;
}

void WeatherRequest::exec()
{
	if (myDest.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: !net weather <city>");
		deleteLater();
		return;
	}

	QString url;

	if ( myDest[1].isNumber() ) {
		url = QString("http://informer.gismeteo.ru/rss/%1.xml").arg(myDest);
	} else {
		url = QString("http://wap.gismeteo.ru/gm/normal/node/search_result/6/?like_field_sname=%1").arg(myDest);
	}

	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.toEncoded());
}

void WeatherRequest::httpRequestFinished(int, bool err)
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
	QByteArray arr = http->readAll();
	if ( arr.isEmpty() ) { // eh? no data? Wait for it.
		//deleteLater();
		return;
	}
	QString data (arr);
	if ( data.startsWith("<rss") ) {
		showForecast( arr );
		return;
	}

	int     fi = data.indexOf("field_index=");
	int     sd = data.indexOf("&sd", fi);
	QString ndx = data.mid(fi+12, (sd-fi-12));

	if ( !ndx[0].isNumber() ) {
		plugin()->reply(stanza(), "Unknown location.");
		deleteLater();
		return;
	}
	http->close();
	QString url = QString("http://informer.gismeteo.ru/rss/%1.xml").arg(ndx);
	QUrl qurl(url);
	http->setHost(qurl.host());
	http->get(qurl.toEncoded());
}

void WeatherRequest::showForecast(QByteArray &data) 
{
	QDomDocument doc; doc.setContent( data );
	QDomNodeList items = doc.elementsByTagName("item");
	QStringList res;
	QDomElement e;
	res << QString("Gismeteo.ru (%1):")
		.arg(items.at(0).toElement()
		.elementsByTagName("title").at(0)
		.firstChild().nodeValue().section(":", 0, 0));

	for (int i = 0; i < items.length(); i++) {
		e = items.at(i).toElement();
		res << e.elementsByTagName("title").at(0).firstChild().nodeValue().section(":",1)
		+": "+ e.elementsByTagName("description").at(0).firstChild().nodeValue();
	}
	if ( items.isEmpty() ) {
		plugin()->reply(stanza(), "no data or unknown location.");
		deleteLater();
		return;
	}

	plugin()->reply(stanza(), res.join("\n").replace("+-", "-"));
	deleteLater();
}

/*
void WeatherRequest::httpRequestRssFinished(int, bool err)
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
	QByteArray arr = http->readAll();
	QString data (arr);
	if ( data.startsWith("<rss") ) {
		showForecast( arr );
	}
	plugin()->reply(stanza(), data);
	deleteLater();
}
*/
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
#include "weatherrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "base/messageparser.h"

//#include <QXml>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QProcess>
#include <QtDebug>
#include <QMutexLocker>
#include <QTextCodec>

WeatherRequest::WeatherRequest(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest = parser.joinBody().trimmed();
	http   = 0;
}

WeatherRequest::~WeatherRequest()
{
	if (http)
		delete http;
}

void WeatherRequest::exec()
{
	if (myDest.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: !net weather <city>");
		deleteLater();
		return;
	}

	QString url;

	if ( myDest[1].isNumber() ) {
		url = QString("http://informer.gismeteo.ru/rss/%1.xml").arg(myDest);
	} else {
		url = QString("http://wap.gismeteo.ru/gm/normal/node/search_result/6/?like_field_sname=%1").arg(myDest);
	}

	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.toEncoded());
}

void WeatherRequest::httpRequestFinished(int, bool err)
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
	QByteArray arr = http->readAll();
	if ( arr.isEmpty() ) { // eh? no data? Wait for it.
		//deleteLater();
		return;
	}
	QString data (arr);
	if ( data.startsWith("<rss") ) {
		showForecast( arr );
		return;
	}

	int     fi = data.indexOf("field_index=");
	int     sd = data.indexOf("&sd", fi);
	QString ndx = data.mid(fi+12, (sd-fi-12));

	if ( !ndx[0].isNumber() ) {
		plugin()->reply(stanza(), "Unknown location.");
		deleteLater();
		return;
	}
	http->close();
	QString url = QString("http://informer.gismeteo.ru/rss/%1.xml").arg(ndx);
	QUrl qurl(url);
	http->setHost(qurl.host());
	http->get(qurl.toEncoded());
}

void WeatherRequest::showForecast(QByteArray &data) 
{
	QDomDocument doc; doc.setContent( data );
	QDomNodeList items = doc.elementsByTagName("item");
	QStringList res;
	QDomElement e;
	res << QString("Gismeteo.ru (%1):")
		.arg(items.at(0).toElement()
		.elementsByTagName("title").at(0)
		.firstChild().nodeValue().section(":", 0, 0));

	for (int i = 0; i < items.length(); i++) {
		e = items.at(i).toElement();
		res << e.elementsByTagName("title").at(0).firstChild().nodeValue().section(":",1)
		+": "+ e.elementsByTagName("description").at(0).firstChild().nodeValue();
	}
	if ( items.isEmpty() ) {
		plugin()->reply(stanza(), "no data or unknown location.");
		deleteLater();
		return;
	}

	plugin()->reply(stanza(), res.join("\n").replace("+-", "-"));
	deleteLater();
}

/*
void WeatherRequest::httpRequestRssFinished(int, bool err)
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
	QByteArray arr = http->readAll();
	QString data (arr);
	if ( data.startsWith("<rss") ) {
		showForecast( arr );
	}
	plugin()->reply(stanza(), data);
	deleteLater();
}
*/
