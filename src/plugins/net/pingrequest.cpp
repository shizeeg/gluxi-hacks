#include "pingrequest.h"
#include "base/baseplugin.h"

#include <QProcess>
#include <QtDebug>

PingRequest::PingRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	proc=0;
}

PingRequest::~PingRequest()
{
	if (proc)
		delete proc;
}

void PingRequest::exec()
{
	proc=new QProcess(this);
	connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
	QString cmd="ping";
	QStringList args;
	args << "-c" << "3" << myDest;
	proc->start(cmd,args);
	if (!proc->waitForStarted())
		qDebug() << "Can't start";
}

void PingRequest::onProcessFinished()
{
	qDebug() << "Process finished";
	QByteArray arr=proc->readAll();
	QString st(arr);
	plugin()->reply(stanza(),st.section("\n\n",0,0));
	deleteLater();
}

void PingRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

