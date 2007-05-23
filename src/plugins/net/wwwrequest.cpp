#include "wwwrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"
#include "proxy/directproxy.h"

#include <QProcess>
#include <QtDebug>
#include <QMutexLocker>
#include <QTextCodec>

WWWRequest::WWWRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& cmd, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	QMutexLocker locker(&deleteMutex);
	myCmd=cmd;
	myDest=dest.trimmed();
	proc=0;
}

WWWRequest::~WWWRequest()
{
	if (proc)
		delete proc;
}

void WWWRequest::launch()
{
	start();
}

void WWWRequest::run()
{
	QMutexLocker locker(&deleteMutex);

	
	if (myDest.toUpper().startsWith("EXP"))
	{
		// We have RegExp
		myDest.remove(0,4);
		myDest=myDest.trimmed();
		if (myDest.startsWith("\n"))
			myDest=myDest.section('\n',1).trimmed();
		myExp=myDest.section('\n',0,0);
		myDest=myDest.section('\n',1);
		if (myExp.isEmpty() || myDest.isEmpty())
		{
			plugin()->reply(stanza(),"RegExp expected");
			wantDelete();
			return;
		}
		if (!QRegExp(myExp).isValid())
		{
			plugin()->reply(stanza(),"RegExp error");
			wantDelete();
			return;
		}
		qDebug() << "|| exp=" << myExp << "  || dest=" << myDest;
	}

	QRegExp exp(myExp);

	DirectProxy proxy(0,0,"");
	proxy.maxSize=2*1024*1024;
	if (myCmd=="HEADERS")
		proxy.headersOnly=true;
	else
		proxy.contentTypes << "text/plain" << "text/html" << "text/xml" << "text/vnd.sun.j2me.app-descriptor";
	
	QStringList cookies;
	QString referer="";
	if (myDest.indexOf("://")<0)
		myDest="http://"+myDest;
	QString res=proxy.fetch(myDest,referer,&cookies,0);
	if (proxy.headersOnly)
	{
		if (proxy.headers.isEmpty())
			plugin()->reply(stanza(),"Error: Can't fetch HTTP headers");
		else
			plugin()->reply(stanza(),QString("Headers for: %1:\n%2")
				.arg(myDest).arg(proxy.headers.join("\n")));
		wantDelete();
		return;
	}
	
	if (!proxy.redirectTo.isEmpty())
	{
		plugin()->reply(stanza(),QString("Location: %1").arg(proxy.redirectTo));
		wantDelete();
		return;
	}

	if(res.isEmpty())
	{
		plugin()->reply(stanza(),"Error: "+proxy.errorString);
		wantDelete();
		return;
	}

	QString enc=proxy.charset;
	if (enc.isEmpty())
		enc="Windows-1251";

	// No recode if we have codepage in META
	QString tag=getValue(res,"<meta[^>]+http-equiv=[\\\"]{0,1}Content-Type[\\\"]{0,1}"
		"[^>]+content=\\\"([^\\\"]+)\\\"[^>]*>");
	QString htmlCharset;
	if (!tag.isEmpty())
		htmlCharset=getValue(tag,"charset=([A-Za-z0-9\\-\\_]+)");

	QByteArray data;
	QTextCodec *codec=QTextCodec::codecForName(enc.toLatin1().data());
	if (codec && htmlCharset.isEmpty())
		data=codec->toUnicode(proxy.lastData).toUtf8();
	else
		data=proxy.lastData;
	
	if (!myExp.isEmpty())
	{
		// Apply regexp
		QString myString(data);
		exp.setMinimal(true);
		if (exp.indexIn(myString)<0)
		{
			plugin()->reply(stanza(), "RegExp don't match");
			wantDelete();
			return;
		}
		QStringList list=exp.capturedTexts();
		if (list.count()<2)
		{
			plugin()->reply(stanza(), "No text captured");
			wantDelete();
			return;
		}
		list.removeFirst();
		list=list.join("\n").split("\n");
		myString.clear();
		for (int i=0; i<list.count(); ++i)
		{
			QString line=removeHtml(list[i]).trimmed();
			if (!line.isEmpty())
			{
				if (!myString.isEmpty())
					myString+="\n";
				myString+=line;
			}
		}

		plugin()->reply(stanza(), myString);
		wantDelete();
		return;
	}
	
	if (proxy.contentType=="text/html")
	{
		proc=new QProcess();
		QStringList args;
		args << "-dump" << "-nolist" << "-display_charset=utf-8" << "-assume_charset=utf-8" << "-stdin";
		proc->start("lynx",args);
		if (!proc->waitForStarted())
		{
			plugin()->reply(stanza(),"Unable to launch lynx -dump");
			wantDelete();
			return;
		}
		proc->write(data);
		proc->closeWriteChannel();
		if (!proc->waitForFinished())
		{
			plugin()->reply(stanza(),"Error: lynx timeout");
			wantDelete();
			return;
		}
		res=QString(proc->readAll());
	}
	else
		res=data;
	plugin()->reply(stanza(),res);

	wantDelete();
}

