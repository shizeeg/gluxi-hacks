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
	myCmd=cmd;
	myDest=dest;
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
	DirectProxy proxy(0,0,"");
	proxy.maxSize=2*1024*1024;
	if (myCmd=="HEADERS")
		proxy.headersOnly=true;
	else
		proxy.contentTypes << "text/plain" << "text/html";
	
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
	plugin()->reply(stanza(),res);

	wantDelete();
}

