#include "asyncrequestlist.h"
#include "baseplugin.h"

#include <gloox/stanza.h>

#include <QtDebug>
#include <QTimer>
#include <QMutexLocker>

AsyncRequestList::AsyncRequestList() :
	QObject(), QList<AsyncRequest*>()
{
	timer=new QTimer(this);
	connect(timer, SIGNAL(timeout()), SLOT(onTimeout()));
	timer->start(60000);
}

AsyncRequestList::~AsyncRequestList()
{
	delete timer;
}

void AsyncRequestList::clear()
{
	int cnt=count();
	for (int i=0; i<cnt; i++)
	{
		delete takeFirst();
	}
}

void AsyncRequestList::removeAt(int i)
{
	AsyncRequest* r=takeAt(i);
	delete r;
}

AsyncRequest* AsyncRequestList::byStanzaId(const QString& id)
{
	QMutexLocker locker(&listMutex_);
	QListIterator<AsyncRequest*> it(*this);
	while (it.hasNext())
	{
		AsyncRequest* r=it.next();
		if (r->stanzaId()==id)
			return r;
	}
	return 0L;
}

AsyncRequest* AsyncRequestList::byStanza(const gloox::Stanza* s)
{
	QString id=QString::fromStdString(s->findAttribute("id"));
	return byStanzaId(id);
}

AsyncRequest* AsyncRequestList::byId(int id)
{
	QMutexLocker locker(&listMutex_);
	QListIterator<AsyncRequest*> it(*this);
	while (it.hasNext())
	{
		AsyncRequest* r=it.next();
		if (r->id()==id)
			return r;
	}
	return 0L;
}

void AsyncRequestList::append(AsyncRequest* itm)
{
	QMutexLocker locker(&listMutex_);
	qDebug() << "New AsyncRequest";
	QList<AsyncRequest*>::append(itm);
	connect(itm, SIGNAL(onDelete(AsyncRequest*)), SLOT(onDelete(AsyncRequest*)), Qt::DirectConnection);
	connect(itm, SIGNAL(onTerminated(AsyncRequest*)), SLOT(onFinished(AsyncRequest*)), Qt::QueuedConnection);
	connect(itm, SIGNAL(onFinished(AsyncRequest*)), SLOT(onFinished(AsyncRequest*)), Qt::QueuedConnection);
}

void AsyncRequestList::onDelete(AsyncRequest* s)
{
	QMutexLocker locker(&listMutex_);
	qDebug() << "AsyncRequestList::onDelete()";
	s->disconnect();
	removeAll(s);
	//	delete s;
}

void AsyncRequestList::onFinished(AsyncRequest* s)
{
	QMutexLocker locker(&listMutex_);
	if (indexOf(s)<0)
	{
		qDebug() << "Thread is not registered";
		return;
	}
	s->deleteMutex.lock();
	qDebug() << "AsyncRequestList::finished/terminated(): " << s->name();
	s->disconnect();
	removeAll(s);
	delete(s);
}

void AsyncRequestList::onTimeout()
{
	QMutexLocker locker(&listMutex_);
	qDebug() << "Testing for old requests";
	// Check for each request for timeout
	QList<AsyncRequest*>::iterator it=begin();
	while (it!=end())
	{
		AsyncRequest* request=(*it);
		if (request->expired())
		{
			QString name;
			if (request->plugin())
				name+=request->plugin()->name()+": ";
			if (request->stanza())
				name+=QString::fromStdString(request->stanza()->body());
			qDebug() << "Expired request: " << name << " running: " << request->isRunning();
			if (!request->isRunning())
			{
				request->disconnect();
				delete request;
				it=erase(it);
			}
			else
			{
				qDebug() << "Skipping running thread";
				++it;	
			}
		}
		else
			++it;
	}
}
