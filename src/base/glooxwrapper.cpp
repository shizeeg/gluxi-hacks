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
	
	QString hostToConnect=storage->getString("account/host");
	if (hostToConnect.isEmpty())
		hostToConnect=storage->getString("account/server");

	myConnection = new gloox::ConnectionTCPClient(myClient, myClient->logInstance(), 
		hostToConnect.toStdString(),
		5222 );
 	myClient->setConnectionImpl(myConnection);

	myClient->disco()->setVersion("GluxiBot (libGLOOX based bot)","0.1",
		version().toStdString());
	myClient->disco()->setIdentity( "client", "bot" );
//	myClient->setAutoPresence( true );
//	myClient->setPriority(storage->getInt("account/priority"));
	myClient->setPresence(gloox::PresenceAvailable, 
		storage->getInt("account/priority"));
	myClient->registerConnectionListener( this );
	myClient->registerMessageHandler(this);
	myClient->registerPresenceHandler(this);
	// Move this 
	myClient->registerIqHandler(this,"http://jabber.org/protocol/muc#admin");
	vcardManager=new gloox::MyVCardManager(myClient);
}

GlooxWrapper::~GlooxWrapper()
{
	delete myClient;
}

void GlooxWrapper::run()
{
	qDebug() << "GlooxWrapper::run()";
	
	int timeout=DataStorage::instance()->getInt("account/keepalive");
	if (timeout<=0)
		timeout=-1;
	else
		timeout*=1000;
	
	// We run connect() in non-blocking mode just because we should be 
	// able to sync our threads
	while (1)
 	{
		myClient->connect(false);
		qDebug() << "Connected";

		// Gluxi loop
		int fd=myConnection->socket();
		int res;
		struct pollfd pfd;
		pfd.fd=fd;
		pfd.events=POLLIN | POLLPRI | POLLERR | POLLNVAL;
		while (1)
		{
			res=poll(&pfd,1, timeout);

			if (res<0)
				break;
			
			if (res==0 && timeout>0)
			{
				gloox::Stanza* s=new gloox::Stanza("iq");
				send(s);
				continue;
			}
			
			if (!res)
				continue;
		
			mutex.lock();
			int err;
			if ((err=myClient->recv(0))!=gloox::ConnNoError)
			{
				qDebug() << QString::fromStdString(myClient->streamErrorText());
				mutex.unlock();
				break;
			}
			mutex.unlock();
		}
		break;
	}
	QCoreApplication::exit(0);
}

void GlooxWrapper::onConnect()
{
	emit sigConnect();
}

void GlooxWrapper::onDisconnect(gloox::ConnectionError /* e */)
{
	emit sigDisconnect();
	// Disconnect
}

bool GlooxWrapper::onTLSConnect( const gloox::CertInfo& )
{
	// Accept any TLS Cert
	return true;
}

void GlooxWrapper::handleMessage(gloox::Stanza* s, gloox::MessageSession*)
{
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


void GlooxWrapper::handleVCard(const gloox::JID &jid, gloox::VCard *vcard)
{
	emit sigVCard(VCardWrapper(jid, vcard)); 
}

void GlooxWrapper::handleVCardResult(VCardContext context, const gloox::JID &jid,
		gloox::StanzaError se)
{
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
	qDebug() << "[OUT] " << s->xml().data();
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

void GlooxWrapper::fetchVCard(const QString& jid) 
{
	QMutexLocker locker(&mutex);
	vcardManager->fetchVCard(gloox::JID(jid.toStdString()),this);
}

void GlooxWrapper::setPresence(gloox::Presence presence, const QString& status,
		int priority)
{
       myClient->setPresence(presence, priority, status.toStdString());
}

void GlooxWrapper::setPresence(const QString& jid, gloox::Presence presence, const QString& status)
{
	gloox::Stanza* st=gloox::Stanza::createPresenceStanza(gloox::JID(jid.toStdString()),status.toStdString(),presence);
	send(st);
}
