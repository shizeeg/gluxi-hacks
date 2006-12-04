#include "tracerouterequest.h"
#include "base/baseplugin.h"

#include <QProcess>
#include <QtDebug>

TraceRouteRequest::TraceRouteRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	proc=0;
}

TraceRouteRequest::~TraceRouteRequest()
{
	if (proc)
		delete proc;
}

void TraceRouteRequest::exec()
{
	proc=new QProcess(this);
	connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
	QString cmd="mtr";
	QStringList args;
	args << "-r" << "-c" << "1" << myDest;
	proc->start(cmd,args);
	if (!proc->waitForStarted())
		qDebug() << "Can't start";
}

void TraceRouteRequest::onProcessFinished()
{
	qDebug() << "Process finished";
	QByteArray arr;
	arr.append(proc->readAllStandardError());
	arr.append(proc->readAllStandardOutput());
	QString st(arr);
	plugin()->reply(stanza(),st);
	emit wantDelete(this);
}

void TraceRouteRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

