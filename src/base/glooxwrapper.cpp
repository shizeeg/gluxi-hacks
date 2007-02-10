#include "glooxwrapper.h"
#include "common.h"
#include "mystanza.h"
#include "datastorage.h"
#include "common.h"

#include <QtDebug>
#include <QCoreApplication>
#include <QMutexLocker>

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/jid.h>

#include <assert.h>
#include <string>
#include <iostream>
#include <poll.h>

GlooxWrapper::GlooxWrapper()
	:QThread()
{
	DataStorage *storage=DataStorage::instance();

	myClient=new gloox::Client(
		storage->getStdString("account/user"),
		storage->getStdString("account/password"),
		storage->getStdString("account/server"),
		storage->getStdString("account/resource"));


	myClient->disco()->setVersion("GluxiBot (libGLOOX based bot)","0.1",
		version().toStdString());
	myClient->disco()->setIdentity( "client", "bot" );
	myClient->setAutoPresence( true );
	myClient->setInitialPriority(storage->getInt("account/priority"));
	myClient->registerConnectionListener( this );
	myClient->registerMessageHandler(this);
	myClient->registerPresenceHandler(this);
	// Move this 
	myClient->registerIqHandler(this,"http://jabber.org/protocol/muc#admin");
}

GlooxWrapper::~GlooxWrapper()
{
	delete myClient;
}

void GlooxWrapper::run()
{
	qDebug() << "GlooxWrapper::run()";
	
	// We run connect() in non-blocking mode just because we should be 
	// able to sync our threads
	myClient->connect(false);
	qDebug() << "Connected";

	// Gluxi loop
	int fd=myClient->fileDescriptor();
	int res;
	struct pollfd pfd;
	pfd.fd=fd;
	pfd.events=POLLIN | POLLPRI | POLLERR | POLLNVAL;
	while (1)
	{
		res=poll(&pfd,1,-1);

		if (res<0)
			break;
		if (!res)
			continue;
		
		mutex.lock();
		if (myClient->recv(0)!=gloox::ConnNoError)
		{
			mutex.unlock();
			break;
		}
		mutex.unlock();
	}
	qDebug() << "GlooxWrapper disconnected";
	emit sigDisconnect();
}

void GlooxWrapper::onConnect()
{
	emit sigConnect();
}

void GlooxWrapper::onDisconnect(gloox::ConnectionError /* e */)
{
	// Disconnect
}

bool GlooxWrapper::onTLSConnect( const gloox::CertInfo& )
{
	// Accept any TLS Cert
	return true;
}

void GlooxWrapper::handleMessage(gloox::Stanza* s)
{
	qDebug() << "Got message";
	emit sigMessage(MyStanza(s));
}

void GlooxWrapper::handlePresence(gloox::Stanza *s)
{
	emit sigPresence(MyStanza(s));
}

bool GlooxWrapper::handleIq(gloox::Stanza* s)
{
	emit sigIq(MyStanza(s));
}

void GlooxWrapper::handleDiscoInfoResult (gloox::Stanza *s, int/* context*/)
{
	emit sigIq(MyStanza(s));
}

void GlooxWrapper::handleDiscoItemsResult (gloox::Stanza *s, int/* context*/)
{
        emit sigIq(MyStanza(s));
}

void GlooxWrapper::handleDiscoError (gloox::Stanza *s, int/* context*/)
{
        emit sigIq(MyStanza(s));
}

bool GlooxWrapper::handleIqID(gloox::Stanza*, int)
{
	return true;
}

// Thread-safe members
void GlooxWrapper::disconnect()
{
	QMutexLocker locker(&mutex);
	qDebug() << "Disconnect request";
	myClient->disconnect();
}

void GlooxWrapper::send(gloox::Stanza* s)
{
	QMutexLocker locker(&mutex);
	myClient->send(s);
}

void GlooxWrapper::registerIqHandler(const QString& service)
{
	QMutexLocker locker(&mutex);
	myClient->registerIqHandler(this,service.toStdString());
}

std::string GlooxWrapper::getID()
{
	QMutexLocker locker(&mutex);
	return myClient->getID();
}

gloox::JID GlooxWrapper::jid()
{
	QMutexLocker locker(&mutex);
	return myClient->jid();
}

