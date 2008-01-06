#ifndef ASYNCREQUESTLIST_H
#define ASYNCREQUESTLIST_H

#include "asyncrequest.h"

#include <QObject>
#include <QList>
#include <QMutex>

class QTimer;

namespace gloox
{
	class Stanza;
}

class AsyncRequestList: public QObject, public QList<AsyncRequest*>
{
	Q_OBJECT
public:
	AsyncRequestList();
	~AsyncRequestList();
	void clear();
	void removeAt(int);
	void append(AsyncRequest* );
	AsyncRequest* byId(int id);
	AsyncRequest* byStanza(const gloox::Stanza *s);
	AsyncRequest* byStanzaId(const QString& req);
private:
	QTimer* timer;
	QMutex listMutex_;
private slots:
	void onTimeout();
	void onDelete(AsyncRequest*);
	void onFinished(AsyncRequest*);
};

#endif

