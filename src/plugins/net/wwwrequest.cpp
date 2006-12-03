#include "wwwrequest.h"
#include "base/baseplugin.h"

#include <QProcess>
#include <QtDebug>

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

void WWWRequest::exec()
{
	proc=new QProcess(this);
	connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
	QString cmd="lynx";
	QStringList args;
	args << "-dump" << "-nolist" << "-display_charset=utf-8" << QString("%1").arg(myDest);
	proc->start(cmd,args);
	if (!proc->waitForStarted())
		qDebug() << "Can't start";
}

void WWWRequest::onProcessFinished()
{
	qDebug() << "Process finished";
	QByteArray arr=proc->readAll();
	QString st(arr);
	plugin()->reply(stanza(),st);
	emit wantDelete(this);
}

void WWWRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

