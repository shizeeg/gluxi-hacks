#include "svnrequest.h"
#include "base/baseplugin.h"
#include "base/common.h"

#include <QProcess>
#include <QtDebug>
#include <QRegExp>

SVNRequest::SVNRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	proc=0;
}

SVNRequest::~SVNRequest()
{
	if (proc)
		delete proc;
}

void SVNRequest::exec()
{
	proc=new QProcess(this);
	connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
	QString cmd="svn";
	QStringList args;
	//svn info http://svn.xmpp.ru/repos/bombus/trunk
	args << "log" << "--non-interactive" << "--limit" << "1" <<"-r" << myDest;
	proc->start(cmd,args);
	if (!proc->waitForStarted())
	{
		plugin()->reply(stanza(),"Can't start svn");
		deleteLater();
	}
}

void SVNRequest::onProcessFinished()
{
	qDebug() << "Process finished";
	QString lines=QString(proc->readAll()).trimmed();
	if (lines.isEmpty())
	{
		plugin()->reply(stanza(), QString("Can't get repositroy info:\n%1")
			.arg(QString(proc->readAllStandardError())));
		deleteLater();
		return;
	}
	if (!lines.endsWith('\n'))
		lines+='\n';
	QStringList list=lines.split('\n');
	if (list.count() && list[0].startsWith("--"))
		list.removeFirst();
	if (list.count() && list[list.count()-1].startsWith("--"))
		list.removeLast();
	int i=0;
	while (i<list.count())
		if (list[i].isEmpty())
			list.removeAt(i);
		else
			i++;
	QString res=QString("SVN info for %1:\n%2").arg(myDest).arg(list.join("\n"));
	
	plugin()->reply(stanza(),res);
	deleteLater();
}

void SVNRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

