#include "googlerequest.h"
#include "base/baseplugin.h"
#include "base/common.h"

#include <QHttp>
#include <QUrl>
#include <QRegExp>
#include <QtDebug>

GoogleRequest::GoogleRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	http=0;
}

GoogleRequest::~GoogleRequest()
{
	if (http)
		delete http;
	http=0;
}

void GoogleRequest::exec()
{
	if (myDest.startsWith("/"))
	{
		QString idx=myDest.section(' ',0,0);
		myDest=myDest.section(' ',1);
		idx.remove(0,1);
		bool ok=true;
		nres=idx.toInt(&ok);
		if (!ok)
		{
			plugin()->reply(stanza(),"result number should be int");
			deleteLater();
			return;
		}
	}
	else
		nres=1;
	if (myDest.isEmpty())
	{
		plugin()->reply(stanza(),"Usage: net google </resnum> [query]");
		deleteLater();
		return;
	}

	int pn=(nres-1)/10;
	nres=(nres-1)%10+1;

	QString url=QString("http://www.google.com/search?q=%1&ie=UTF-8&oe=UTF-8&start=%2").arg(myDest).arg(pn);
	QUrl qurl(url);
	http=new QHttp(qurl.host());
	connect(http,SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
	http->get(qurl.toEncoded());
}

void GoogleRequest::httpRequestFinished(int, bool err)
{
	if (err || http->lastResponse().statusCode()!=200)
	{
		plugin()->reply(stanza(),"Failed to Google: "+http->lastResponse().reasonPhrase());
		deleteLater();
		return;
	}
	QString buf=http->readAll();
	QRegExp exp("<div class=g><h2 class=r>(.*)</td></tr></table></div>");
	exp.setMinimal(TRUE);
	QString res;
	int ps=0;

	while (nres>0 && ((ps=exp.indexIn(buf,ps))>=0))
	{
		QStringList lst=exp.capturedTexts();
		ps+=exp.matchedLength();
		res=lst[1];
		--nres;
	}
	if (nres || res.isEmpty())
	{
		plugin()->reply(stanza(), "Can't parse google");
		deleteLater();
		return;
	}
	QString url=removeHtml(getValue(res,"<a href=\\\"(.*)\\\" class=l>"));
	QString subj=removeHtml(getValue(res,"<a[^>]*class=l>(.*)</a>"));
	QString body=removeHtml(getValue(res,"<tr><td class=j><font size=-1>(.*)<br><span class=a>"));
	plugin()->reply(stanza(),QString("%1\n%2\n%3").arg(subj).arg(body).arg(url));
	deleteLater();
}

