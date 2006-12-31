#ifndef WEBSTATUSTHREAD_H
#define WEBSTATUSTHREAD_H

#include <QThread>
#include <QMutex>

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
};

#endif

