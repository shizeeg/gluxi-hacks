#include "logger.h"

#include <sys/types.h>
#include <sys/wait.h>

#include <QMutexLocker>
#include <QDateTime>
#include <QStringList>

Logger* Logger::instance_=0;

void myMessageOutput(QtMsgType type, const char *msg)
{
	Logger::instance()->log(type, QString(msg));
}

void onHUP(int)
{
	fprintf(stderr, "Got SIGHUP... ");
	if (Logger::instance())
		Logger::instance()->hup();
}

Logger::Logger(const QString& fileName)
{
	if (instance_==0)
	{
		instance_=this;
		printf("----------------------xzfsdp-gsdgs ------------- \n");
		qInstallMsgHandler(myMessageOutput);
		struct sigaction act;
		memset(&act, 0, sizeof(act));
		act.sa_handler=onHUP;
		act.sa_flags=SA_RESTART;
		sigaction(SIGHUP, &act, NULL);
	}
	fileName_=fileName;
	hup_=false;
	open();
}

Logger::~Logger()
{
	close();
}

Logger* Logger::instance()
{
	return instance_;
}

void Logger::open()
{
	file_.setFileName(fileName_);
	file_.open(QIODevice::ReadWrite | QIODevice::Append);
	stream_.setDevice(&file_);
}

void Logger::close()
{
	stream_.flush();
	file_.close();
}

void Logger::hup()
{
	hup_=true;
}

void Logger::log(QtMsgType type, QString msg)
{
	QMutexLocker locker(&mutex_);

	if (hup_)
	{
		hup_=false;
		close();
		open();
	}

	QString dateTime=QDateTime::currentDateTime().toString(Qt::ISODate);
	QString typeStr;
	switch (type)
	{
	case QtDebugMsg:
		typeStr="DEBUG";
		break;
	case QtWarningMsg:
		typeStr="WARN";
		break;
	case QtCriticalMsg:
		typeStr="CRITICAL";
		break;
	case QtFatalMsg:
		typeStr="FATAL";
		break;
	}
	QStringList messages=msg.split('\n');
	foreach(QString message, messages)
	{
		QString logMsg=QString("[%1] %2: %3").arg(dateTime).arg(typeStr).arg(message);
		stream_ << logMsg << endl;
		fprintf(stderr, "%s\n",logMsg.toLocal8Bit().data());
	}
	stream_.flush();
}

