#include "wwwrequest.h"
#include "base/baseplugin.h"
#include "proxy/directproxy.h"

#include <QProcess>
#include <QtDebug>
#include <QMutexLocker>
#include <QTextCodec>

WWWRequest::WWWRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
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
	proxy.contentTypes << "text/plain" << "text/html";
	QStringList cookies;
	QString referer="";
	QString res=proxy.fetch(myDest,referer,&cookies,0);
	if(res.isEmpty())
	{
		plugin()->reply(stanza(),"Error: "+proxy.errorString);
		wantDelete();
		return;
	}
	QString enc=proxy.charset;
	if (enc.isEmpty())
		enc="Windows-1251";

	QByteArray data;
	QTextCodec *codec=QTextCodec::codecForName(enc.toLatin1().data());
	if (codec)
		data=codec->toUnicode(proxy.lastData).toUtf8();
	else
		data=proxy.lastData;
	
	proc=new QProcess();
	QStringList args;
	args << "-dump" << "-nolist" << "-display_charset=utf-8" << "-stdin";
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

