#include "nslookuprequest.h"
#include "base/baseplugin.h"

#include <QProcess>
#include <QtDebug>

NslookupRequest::NslookupRequest(BasePlugin *plugin, gloox::Stanza *from, const QString& dest)
	:AsyncRequest(-1, plugin, from, 300)
{
	myDest=dest;
	proc=0;
}

NslookupRequest::~NslookupRequest()
{
	if (proc)
		delete proc;
}

void NslookupRequest::exec()
{
	proc=new QProcess(this);
	connect(proc, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onProcessFinished()));
	connect(proc, SIGNAL(stateChanged(QProcess::ProcessState)), SLOT(onStateChanged(QProcess::ProcessState)));
	QString cmd="nslookup";
	QStringList args = myDest.split(' ');

	proc->start(cmd,args);
	if (!proc->waitForStarted())
		qDebug() << "Can't start";
}

void NslookupRequest::onProcessFinished()
{
	qDebug() << "Process finished";
	QByteArray arr;
	arr.append(proc->readAllStandardError());
	arr.append(proc->readAllStandardOutput());
	QString st(arr);
	plugin()->reply(stanza(),st.trimmed());
	deleteLater();
}

void NslookupRequest::onStateChanged ( QProcess::ProcessState newState )
{
	qDebug() << "STCH: " << newState;	
}

