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
	args << "info" <<  myDest;
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
	QString rev=getValue(lines,"\\Revision: ([0-9]+)\\n");
	QString author=getValue(lines,"\\nLast Changed Author: (.+)\\n");
	QString date=getValue(lines,"\\nLast Changed Date: (.+)\\n");
	if (rev.isEmpty())
	{
		plugin()->reply(stanza(),"Can't parse svn output:\n");
		deleteLater();
		return;
	}
	QString res=QString("SVN info for %1:\nRevision: %2").arg(myDest).arg(rev);
	if (!author.isEmpty())
		res+=QString(" by %1").arg(author);
	if (!date.isEmpty())
		res+=QString(" (%1)").arg(date);
	
	plugin()->reply(stanza(),res);
	deleteLater();
}

void SVNRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

