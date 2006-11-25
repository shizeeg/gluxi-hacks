#ifndef ASYNCREQUESTLIST_H
#define ASYNCREQUESTLIST_H

#include "asyncrequest.h"

#include <QList>

class AsyncRequestList: public QList<AsyncRequest*>
{
public:
	void clear();
	void removeAt(int);
	AsyncRequest* byId(const QString&);
};

#endif

