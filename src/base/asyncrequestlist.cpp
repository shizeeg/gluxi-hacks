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

AsyncRequest* AsyncRequestList::byId(const QString& id)
{
	int cnt;
	QListIterator<AsyncRequest*> it(*this);
	while (it.hasNext())
	{
		AsyncRequest* r=it.next();
		if (r->id()==id)
			return r;
	}
	return 0L;
}

