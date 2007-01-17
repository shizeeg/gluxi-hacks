#include "asyncrequestlist.h"

#include <gloox/stanza.h>

#include <QtDebug>
#include <QTimer>

AsyncRequestList::AsyncRequestList()
	:QObject(), QList<AsyncRequest*>()
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
	qDebug() << "New AsyncRequest";
	QList<AsyncRequest*>::append(itm);
	connect(itm, SIGNAL(onDelete(AsyncRequest*)), SLOT(onDelete(AsyncRequest*)));
	connect(itm, SIGNAL(onWantDelete(AsyncRequest*)), SLOT(onWantDelete(AsyncRequest*)));
}

void AsyncRequestList::onDelete(AsyncRequest* s)
{
	qDebug() << "AsyncRequestList::onDelete()";
	s->disconnect();
	removeAll(s);
//	delete s;
}

void AsyncRequestList::onWantDelete(AsyncRequest* s)
{
	s->deleteMutex.lock();
	qDebug() << "AsyncRequestList::onWantDelete()";
	s->disconnect();
	removeAll(s);
	s->terminate();
	s->wait(1);
	delete(s);
}

void AsyncRequestList::onTimeout()
{
	qDebug() << "Testing for old requests";
	// Check for each request for timeout
	QList<AsyncRequest*>::iterator it=begin();
	while (it!=end())
	{
		if ((*it)->expired())
		{
			(*it)->disconnect();
			delete *it;
			it=erase(it);
		}
		else
			++it;
	}
}

