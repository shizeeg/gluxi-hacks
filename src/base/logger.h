#ifndef LOGGER_H_
#define LOGGER_H_

#include <QString>
#include <QMutex>
#include <QFile>
#include <QTextStream>

class Logger
{
public:
	Logger(const QString& fileName);
	virtual ~Logger();
	void log(QtMsgType type, QString msg);
	void hup();
	static Logger* instance();
private:
	static Logger* instance_;
	bool hup_;
	QString fileName_;
	QFile file_;
	QTextStream stream_;
	QMutex mutex_;
	void close();
	void open();
};

#endif /*LOGGER_H_*/
