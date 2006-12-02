#include "glooxwrapper.h"
#include "common.h"
#include "mystanza.h"
#include "datastorage.h"
#include "common.h"

#include <QtDebug>
#include <QCoreApplication>

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/jid.h>

#include <assert.h>
#include <string>
#include <iostream>

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
	myClient->connect();
	qDebug() << "GlooxWrapper disconnected";
	QCoreApplication::exit(0);
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

bool GlooxWrapper::handleIqID(gloox::Stanza*, int)
{
	return true;
}

