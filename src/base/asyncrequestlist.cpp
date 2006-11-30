#include "asyncrequestlist.h"

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

