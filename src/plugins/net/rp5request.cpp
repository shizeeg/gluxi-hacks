/***************************************************************************
 *   Copyright (C) 2010 by Sidgyck                                         *
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
#include "rp5request.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "base/messageparser.h"

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

Rp5Request::Rp5Request(BasePlugin *plugin, gloox::Stanza *from, MessageParser& parser)
	:AsyncRequest(-1, plugin, from, 300)
{
	parser.back(1);
	myCmd  = parser.nextToken().toUpper();
	myDest = parser.joinBody().trimmed();
	http   = 0;
}

Rp5Request::~Rp5Request()
{
	if (http)
		delete http;
}

void Rp5Request::exec()
{
	if (myDest.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: !net rp5weather <city>");
		deleteLater();
		return;
	}

	QString url;
	if ( myDest[1].isNumber() )
	{
		if (myCmd.endsWith("EX"))
			url = QString("http://rp5.ru/xml/%1/ru").arg(myDest);
		else
			url = QString("http://rp5.ru/rss/%1/ru").arg(myDest);
	}
	else 
	{
		url = QString("http://rp5.ru/search.php?lang=ru&txt=%1")
			.arg(urlEncode(myDest, "cp1251").toUpper());
		qDebug() << "[WEATHER]: url(" << url << ");\n";
	}

	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(url);
}

void Rp5Request::httpRequestFinished(int, bool err)
{
	QHttpResponseHeader lastResponse = http->lastResponse();
	if (err || lastResponse.statusCode()!=200 ) 
	{
		if (lastResponse.statusCode()==301 || lastResponse.isValid())
		{
			http->close();
			QUrl qurl(lastResponse.value("location"));
			http->setHost(qurl.host());
			http->get(qurl.toEncoded());
		}
		else 
		{
			plugin()->reply(stanza(), QString("Failed to fetch from %1: ")
				.arg(myDest) +http->lastResponse().reasonPhrase());
			deleteLater();
		}
		return;
	}


	QTextCodec *codec = QTextCodec::codecForName("cp1251");
	QByteArray pageData = http->readAll();
	QByteArray data = codec->toUnicode(pageData).toUtf8();
	if (data.startsWith("<?xml"))
	{
		if (myCmd.endsWith("EX"))
			showForecastEx(pageData);
		else
			showForecast(pageData);
		return;
	}

	int cnt = getValue(data, "<p class=\"stinfo\">(.*)</p>")
		.section(':', 1).trimmed().toInt();
	if (cnt <= 0 && !http->hasPendingRequests())
	{
		plugin()->reply(stanza(), "No locations found");
		deleteLater();
		return;
	}

	QRegExp exp("<tr height=\"25\"><td><a href=\"/(.*)</td></tr>");
	exp.setMinimal(true);
	QStringList list;
	int pos = 0;
	while ((pos = exp.indexIn(data, pos)) != -1)
	{
		list << removeHtml(exp.cap(1)
				   .replace("</td><td>", " | ")
				   .replace("/ru\">", ") "));
		pos += exp.matchedLength();
	}
	if (list.count() > 1)
	{
		plugin()->reply(stanza(), QString("%1 locations found:\n(").arg(cnt) + list.join("\n("));
		deleteLater();
		return;
	}
	else if (list.count() == 1)
	{
		QString url, strCode = list.at(0).section(')', 0, 0);
		if (myCmd.endsWith("EX"))
			url = QString("http://rp5.ru/xml/%1/ru").arg(strCode);
		else
			url = QString("http://rp5.ru/rss/%1/ru").arg(strCode);

		http->close();
		QUrl qurl(url);
		http->setHost(qurl.host());
		http->get(qurl.toEncoded());
	}
}

void Rp5Request::showForecast(QByteArray &data) 
{
	QDomDocument doc; doc.setContent( data );
	QDomNodeList items = doc.elementsByTagName("item");
	QStringList res;
	QDomElement e;
	res << QString("rp5.ru (%1):")
		.arg(items.at(0).toElement()
		.elementsByTagName("title").at(0)
		.firstChild().nodeValue().section(":", 0, 0));

	for (int i = 0; i < items.length(); i++) 
	{
		e = items.at(i).toElement();
		res << e.elementsByTagName("title").at(0).firstChild().nodeValue().section(":",1)
		+": "+ e.elementsByTagName("description").at(0).firstChild().nodeValue();
	}
	if (items.isEmpty()) {
		plugin()->reply(stanza(), "no data or unknown location.");
		deleteLater();
		return;
	}

	plugin()->reply(stanza(), res.join("\n"));//.replace("+-", "-"));
	deleteLater();
}

void Rp5Request::showForecastEx(QByteArray &data)
{
	QDomDocument doc; doc.setContent(data);
	QDomNodeList items = doc.elementsByTagName("timestep");
	QStringList res;
	QString strName = doc.elementsByTagName("point_name").at(0).firstChild().nodeValue();

	for (int i = 0; i < items.length(); i++) 
		res << parseXml(items.at(i).toElement());

	plugin()->reply(stanza(), strName + ":\n" + res.join("\n"));
}

QString Rp5Request::parseXml(QDomElement e)
{
	QStringList falls; falls
        << "без осадков" << "дождь"
	<< "дождь со снегом" << "снег";
 
	int ntemp = e.elementsByTagName("temperature").at(0).firstChild().nodeValue().toInt();
	int nft = e.elementsByTagName("falls").at(0).firstChild().nodeValue().toInt();

	return QString("%1: %2, %3, ветер %4, %5 м/c, облачность %6%, атм.д %7мм рт.с, влажность %8%")
		.arg(QDateTime::fromString(e.elementsByTagName("datetime")
		     .at(0).firstChild().nodeValue(), "yyyy-M-dd hh:mm").toString("ddd, d MMM hh:mm"))
		.arg(((ntemp > 0) ? "+":"") + QString::number(ntemp))
		.arg(falls[nft])
		.arg(e.elementsByTagName("wind_direction").at(0).firstChild().nodeValue())
		.arg(e.elementsByTagName("wind_velocity").at(0).firstChild().nodeValue())
		.arg(e.elementsByTagName("cloud_cover").at(0).firstChild().nodeValue())
		.arg(e.elementsByTagName("pressure").at(0).firstChild().nodeValue())
		.arg(e.elementsByTagName("humidity").at(0).firstChild().nodeValue());
}
