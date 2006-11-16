#include "glooxbot.h"
#include "datastorage.h"

/*#include "plugins/coreplugin.h"
#include "plugins/miscplugin.h"
#include "plugins/adminplugin.h"
#include "plugins/mucplugin.h"
#include "plugins/aliasplugin.h"*/

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/jid.h>

#include <assert.h>
#include <string>
#include <iostream>

GlooxBot::GlooxBot()
{
	DataStorage *storage=DataStorage::instance();
	storage->connect();

	myClient=new gloox::Client(
		storage->getStdString("account/user"),
		storage->getStdString("account/password"),
		storage->getStdString("account/server"),
		storage->getStdString("account/resource"));


	myClient->disco()->setVersion("GlooxBot","0.1","libGLOOX based bot");
	myClient->disco()->setIdentity( "client", "bot" );
	myClient->setAutoPresence( true );
	myClient->setInitialPriority(storage->getInt("account/priority"));
	myClient->registerConnectionListener( this );
	myClient->registerMessageHandler(this);
	myClient->registerPresenceHandler(this);
	myClient->registerIqHandler(this,"http://jabber.org/protocol/muc#admin");

	myOwners.append("dion@jabber.inhex.net");

// 	confList.push_back(new Conference(this, myClient, "motofan@conference.jabber.inhex.net","gloox"));
//	confList.push_back(new Conference(this, myClient, "botzone@conference.jabber.ru","gloox"));
// 	confList.push_back(new Conference(this, myClient, "bombus@conference.jabber.ru","gloox"));

/*	myPlugins.append(new CorePlugin(this));
	myPlugins.append(new MiscPlugin(this));
	myPlugins.append(new AdminPlugin(this));
	myPlugins.append(new MucPlugin(this));
	myPlugins.append(new AliasPlugin(this));*/
}

GlooxBot::~GlooxBot()
{
	delete myClient;
}

void GlooxBot::run()
{
	myClient->connect();
}

// void GlooxBot::join(Conference *c)
// {
// 	connect(c, SIGNAL(needJoin( const QString&, const QString& )),
// 		SLOT(needJoin( const QString&, const QString& )));
// 	gloox::Stanza *st=gloox::Stanza::createPresenceStanza(gloox::JID(c->jid()));
// 	myClient->send(st);
// }

void GlooxBot::onConnect()
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
// 	std::cout << "Joining..." << std::endl;
//
// 	for (std::list<Conference*>::iterator it=confList.begin(); it!=confList.end(); it++)
// 	{
// 		join(*it);
// 	}
// 	std::cout << "Joined" << std::endl;
}

void GlooxBot::onDisconnect(gloox::ConnectionError /* e */)
{}

bool GlooxBot::onTLSConnect( const gloox::CertInfo& )
{
	return true;
}

void GlooxBot::handleMessage(gloox::Stanza* s)
{
// 	std::cout <<"[MESSAGE] "<< s->from().full() << " to " << s->to().full() << " " << s->body() << std::endl << std::endl;
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

bool GlooxBot::isMyMessage(gloox::Stanza *s)
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

void GlooxBot::handlePresence( gloox::Stanza *s	)
{
// 	std::cout <<"[PRESENCE] " << s->from().full() << " to " << s->to().full() << " XMLNS=" << s->xmlns()
// 	 << " ID=" << s->id()
// 	 << " STATUS=" << s->status()
// 	 << " PR=" << s->priority()
// 	 << std::endl;
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

bool GlooxBot::handleIq(gloox::Stanza* s)
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

bool GlooxBot::handleIqID(gloox::Stanza*, int)
{
	return true;
}

// void GlooxBot::needJoin(const QString& conf, const QString& nick)
// {
// 	Conference *c=new Conference(this, myClient, conf.toStdString(), nick.toStdString());
// 	confList.push_back(c);
// 	join(c);
// }

QList<int> GlooxBot::getStorage(gloox::Stanza*s)
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
