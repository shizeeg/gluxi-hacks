#include "webstatusthread.h"
#include "base/datastorage.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <errno.h>

#include <QMutexLocker>
#include <QtDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

WebStatusThread::WebStatusThread()
	:QThread()
{
	socketName=DataStorage::instance()->getString("webstatus/socket");
	shouldWork=1;
	db=QSqlDatabase::cloneDatabase(QSqlDatabase::database(),"webStatus");
	if (!db.open())
	{
		qDebug() << "Unable to open DB for webstatus: " << db.lastError().text();
	}
}

WebStatusThread::~WebStatusThread()
{
	if (!socketName.isEmpty())
		unlink(socketName.toLocal8Bit().data());
	db.close();
}

void WebStatusThread::run()
{
	struct sockaddr_un sa;
	unlink(socketName.toLocal8Bit().data());
	strcpy(sa.sun_path, socketName.toLocal8Bit().data());
	sa.sun_family=AF_UNIX;
	int fd_srv=socket(AF_UNIX, SOCK_STREAM, 0);
	bind(fd_srv, (struct sockaddr*)&sa, sizeof(sa));
	listen(fd_srv, SOMAXCONN);

	if (chmod(socketName.toLocal8Bit().data(), 0777)!=0)
	{
		perror("Unable to chmod for socket: ");
	}

	int sw;
	char buf[1024];
	int bsize;
	fd_set rfds;
	struct timeval tv;
	while (1)
	{
		mutex.lock();
		sw=shouldWork;
		mutex.unlock();
		if (!sw)
			break;
		int fd_client=accept(fd_srv, NULL, 0);
		if (fd_client<0) continue;
		
		bsize=0;

		bool requestFinished=false;
		while (1)
		{
			tv.tv_sec=3;
			tv.tv_usec=0;
			FD_ZERO(&rfds);
			FD_SET(fd_client, &rfds);
			int res=select(fd_client+1, &rfds, NULL, NULL, &tv);
			if (res == -1)
				break;
			if (!res)
			{
				qDebug() << "WebStatus client timeout";
				break;
			}
			int nr=read(fd_client,buf+bsize,sizeof(buf)-1-bsize);
			if (nr<0)
				break;
			for (int i=bsize; i<bsize+nr; i++)
				if (buf[i]==13 || buf[i]==10)
				{
					requestFinished=true;
					buf[i]=0;
					break;
				}
			bsize+=nr;
			if (requestFinished) break;
		}
		if (!requestFinished)
		{
			close(fd_client);
			continue;
		}

		buf[bsize]=0;
		QString request(buf);
		
		QString answer;

		if (request.indexOf('\"')<0 && request.indexOf('\'')<0 
			&& request.startsWith("id=") && request.indexOf('&')<0)
		{
			request.remove(0,3); // Delete id=
			qDebug() << request;
			QSqlQuery query(db);
			query.prepare("SELECT LOWER(status) FROM webstatus WHERE hash=?");
			query.addBindValue(request);
			if (query.exec() && query.next())
			{
				QString status=query.value(0).toString();
				qDebug() << "st=" << status;
				query.prepare(QString("SELECT %1 FROM webstatus WHERE hash=?").arg(status));
				query.addBindValue(request);
				if (query.exec() && query.next())
					answer=query.value(0).toString();
			}
			else
				qDebug() << query.lastError().text();
		}
		answer+='\n';
		qDebug() << answer;
		write(fd_client,answer.toLocal8Bit().data(), answer.toLocal8Bit().size());
		close(fd_client);
	}
}

void WebStatusThread::stop()
{
	QMutexLocker locker(&mutex);
	shouldWork=0;
}

