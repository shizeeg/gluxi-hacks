#include "gluxibot.h"
#include "datastorage.h"
#include "pluginloader.h"
#include "common.h"

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/jid.h>

#include <assert.h>
#include <string>
#include <iostream>

GluxiBot::GluxiBot()
{
	DataStorage *storage=DataStorage::instance();
	storage->connect();

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
	myClient->registerIqHandler(this,"http://jabber.org/protocol/muc#admin");

	myOwners.append("dion@jabber.inhex.net");
	PluginLoader::loadPlugins(&myPlugins,this);
}

GluxiBot::~GluxiBot()
{
	delete myClient;
}

void GluxiBot::run()
{
	myClient->connect();
}

void GluxiBot::onConnect()
{
	std::cout << "Connected" << std::endl;

	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		plugin->onConnect();
	}
	std::cout << "onConnect() sent" << std::endl;
}

void GluxiBot::onDisconnect(gloox::ConnectionError /* e */)
{}

bool GluxiBot::onTLSConnect( const gloox::CertInfo& )
{
	return true;
}

void GluxiBot::handleMessage(gloox::Stanza* s)
{
	std::cout << s->xml() << std::endl << std::endl;

	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	bool parsed=false;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandleMessage(s))
		{
			parsed=plugin->onMessage(s);
			if (parsed) break;
		}
	}
}

bool GluxiBot::isMyMessage(gloox::Stanza *s)
{
	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->isMyMessage(s))
			return true;
	}
	return false;
}

void GluxiBot::handlePresence( gloox::Stanza *s	)
{
	std::cout << s->xml() << std::endl << std::endl;

	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandlePresence(s))
			plugin->onPresence(s);
	}
}

bool GluxiBot::handleIq(gloox::Stanza* s)
{
	std::cout << s->xml() << std::endl << std::endl;

	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandleIq(s))
			plugin->onIq(s);
	}
	return true;
}

bool GluxiBot::handleIqID(gloox::Stanza*, int)
{
	return true;
}

QList<int> GluxiBot::getStorage(gloox::Stanza*s)
{
	QList<int> res;
	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		int st=plugin->getStorage(s);
		if (st)
		{
			res << plugin->id();
			res << st;
			return res;
		}
	}
	return res;
}

QString GluxiBot::getJID(gloox::Stanza* s, const QString& nick)
{
	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		QString jid=plugin->getJID(s,nick);
		if (!jid.isEmpty())
		{
			return jid;
		}
	}
	return QString::null;
}

QString GluxiBot::JIDtoNick(const QString& jid)
{
	QListIterator<BasePlugin*> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		QString nick=plugin->JIDtoNick(jid);
		if (!nick.isEmpty())
		{
			return nick;
		}
	}
	return QString::null;
}

