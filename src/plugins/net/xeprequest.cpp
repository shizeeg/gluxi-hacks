#include "xeprequest.h"
#include "base/baseplugin.h"

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QtDebug>

XepRequest::XepRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	http=0;
}

XepRequest::~XepRequest()
{
	if (http)
		delete http;
	http=0;
}

void XepRequest::exec()
{
	bool ok=true;
	int tmp=myDest.toInt(&ok);
	if (!ok)
	{
		plugin()->reply(stanza(),"Usage: net xep [XEP NUMBER]");
		deleteLater();
		return;
	}
	xep=QString::number(tmp).rightJustified(4,QChar('0'));
	url=QString("http://xmpp.org/extensions/xep-%1.html").arg(xep);
	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.path());
}

void XepRequest::httpRequestFinished(int, bool err)
{
	if (err || http->lastResponse().statusCode()!=200)
	{
		plugin()->reply(stanza(),"Failed to fetch XEP: "+http->lastResponse().reasonPhrase());
		qDebug() << http->lastResponse().toString();
		deleteLater();
		return;
	}
	QString buf=http->readAll();
	QRegExp exp("<title>([^<]*)</title>.*<meta name=\\\"DC\\.Description\\\" content=([^>]*)>");

	if (exp.indexIn(buf)<0)
	{
		plugin()->reply(stanza(),"Can't parse: "+url);
		deleteLater();
		return;
	}
	QStringList lst=exp.capturedTexts();
	QString reply=QString("%1\n%2\n%3").arg(lst[1]).arg(lst[2]).arg(url);
	plugin()->reply(stanza(),reply);
	deleteLater();
}

