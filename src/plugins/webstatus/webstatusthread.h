#ifndef WEBSTATUSTHREAD_H
#define WEBSTATUSTHREAD_H

#include <QThread>
#include <QMutex>
#include <QSqlDatabase>

class WebStatusThread: public QThread
{
	Q_OBJECT
public:
	WebStatusThread();
	~WebStatusThread();
	void stop();
protected:
	void run();
private:
	int shouldWork;
	QString socketName;
	QMutex mutex;
	QSqlDatabase db;
};

#endif

