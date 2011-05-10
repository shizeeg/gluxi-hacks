#include "gluxibot.h"
#include "mystanza.h"
#include "glooxwrapper.h"
#include "datastorage.h"
#include "pluginloader.h"
#include "asyncrequestlist.h"
#include "common.h"
#include "baseplugin.h"
#include "rolelist.h"
#include "vcardwrapper.h"
#include "logger.h"
#include "config/abstractconfigurator.h"
#include "disco/rootdiscohandler.h"
#include "disco/identityitem.h"
#include "disco/featureitem.h"

#include <QtDebug>
#include <QMetaType>
#include <QCoreApplication>

#include <gloox/client.h>
#include <gloox/disco.h>
#include <gloox/jid.h>

#include <assert.h>
#include <string>
#include <iostream>

GluxiBot::GluxiBot()
	:QObject()
{
	DataStorage *storage=DataStorage::instance();
	QString logFile=storage->getString("log/file");
	if (!logFile.isEmpty())
	{
		new Logger(logFile);
	}
	qRegisterMetaType<MyStanza>("MyStanza");
	qRegisterMetaType<VCardWrapper>("VCardWrapper");
	myGloox=new GlooxWrapper();
	connect(myGloox, SIGNAL(sigConnect()),
		this, SLOT(onConnect()),Qt::QueuedConnection);
	connect(myGloox, SIGNAL(sigDisconnect()),
		this, SLOT(onDisconnect()), Qt::QueuedConnection);
	connect(myGloox, SIGNAL(sigMessage(const MyStanza&)),
		this, SLOT(handleMessage(const MyStanza&)), Qt::QueuedConnection);
	connect(myGloox, SIGNAL(sigPresence(const MyStanza&)),
		this, SLOT(handlePresence(const MyStanza&)), Qt::QueuedConnection);
	connect(myGloox, SIGNAL(sigIq(const MyStanza&)),
		this, SLOT(handleIq(const MyStanza&)), Qt::QueuedConnection);
	connect(myGloox, SIGNAL(sigVCard(const VCardWrapper&)),
			this, SLOT(handleVCard(const VCardWrapper&)), Qt::QueuedConnection);

	myRoles=new RoleList();
	myRoles->insert(storage->getString("access/owner"),ROLE_BOTOWNER);
	int i=1;
	while (1)
	{
		QString tmp=storage->getString(QString("access/owner%1").arg(i++));
		if (tmp.isEmpty())
			break;
		myRoles->insert(tmp, ROLE_BOTOWNER);
	}
/*	myOwners.append(storage->getString("access/owner"));*/

	rootDiscoHandler_=new RootDiscoHandler(this);

	myAsyncRequests=new AsyncRequestList();
	PluginLoader::loadPlugins(&myPlugins,this);
	qSort(myPlugins.begin(),myPlugins.end());

	myJid = QString("%1@%2")
		.arg(storage->getString("account/user"))
		.arg(storage->getString("account/server"));

	// Launch Gloox thread
	myGloox->start();
}

GluxiBot::~GluxiBot()
{
	delete myGloox;
}

/*
gloox::Client* GluxiBot::client()
{
	return myGloox->client();
}
*/

void GluxiBot::onConnect()
{
	std::cout << "Connected" << std::endl;

	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		plugin->onConnect();
	}
	std::cout << "onConnect() sent" << std::endl;
}

void GluxiBot::onDisconnect()
{
	std::cout << "Disconnected" << std::endl;

	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		plugin->onDisconnect();
	}
	std::cout << "onDisconnect() sent" << std::endl;
}

void GluxiBot::processMessage(gloox::Stanza *s, const QStringList& flags)
{
	qDebug() << "[IN ] " << s->xml().data();

	// TODO: Implement Async message handler by ID if required
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	bool parsed=false;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandleMessage(s, flags))
		{
			parsed=plugin->onMessage(s, flags);
			if (parsed) break;
		}
	}
}

void GluxiBot::handleMessage(const MyStanza& st)
{
	static const QStringList emptyList;

	gloox::Stanza *s=st.stanza();
	processMessage(s, emptyList);
}

bool GluxiBot::isMyMessage(gloox::Stanza *s)
{
	QListIterator<PluginRef> it(myPlugins);
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

void GluxiBot::handlePresence(const MyStanza& st)
{
	gloox::Stanza *s=st.stanza();

	qDebug() << "[IN ] " << s->xml().data();

	BasePlugin *plugin;
	plugin=pluginByStanzaId(s);
	if (plugin)
	{
		plugin->onPresence(s);
		return;
	}

	QListIterator<PluginRef> it(myPlugins);
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandlePresence(s))
			plugin->onPresence(s);
	}
}

void GluxiBot::handleIq(const MyStanza& st)
{
	static QString lastIqErrorId;
	gloox::Stanza *s=st.stanza();

	qDebug() << "[IN ] " << s->xml().data();

	gloox::Stanza* iqReply=rootDiscoHandler_->handleDiscoRequest(s);
	if (iqReply)
	{
		myGloox->send(iqReply);
		return;
	}

	BasePlugin *plugin;
	plugin=pluginByStanzaId(s);
	if (plugin)
	{
		if (plugin->onIq(s))
			return;
	}

	QListIterator<PluginRef> it(myPlugins);
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		if (plugin->canHandleIq(s))
		{
			if (plugin->onIq(s))
			{
				qDebug() << "IQ handled by " << plugin->name();
				return;
			}
		}
	}
	if (s->findAttribute("type")!="error")
	{
		// Workaround Talisman bug, that sends strange replies to <iq type='error' /> stanzas
		QString sid=QString::fromStdString(s->id());
		if (lastIqErrorId==sid)
			return;
		lastIqErrorId=sid;

		gloox::Stanza* errStanza=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqError,s->xmlns());
		client()->send(errStanza);
	}
}

void GluxiBot::handleVCard(const VCardWrapper& vcard)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		plugin->onVCard(vcard);
	}
}

StorageKey GluxiBot::getStorage(gloox::Stanza*s)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		StorageKey key=plugin->getStorage(s);
		if (key.isValid())
			return key;
	}
	return StorageKey();
}

QString GluxiBot::getJID(gloox::Stanza* s, const QString& nick, bool realJid)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		QString jid=plugin->getJID(s,nick, realJid);
		if (!jid.isEmpty())
		{
			return jid;
		}
	}
	return QString::null;
}

QString GluxiBot::JIDtoNick(const QString& jid)
{
	QListIterator<PluginRef> it(myPlugins);
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

void GluxiBot::customEvent(QEvent *event)
{
        if (event->type()==QEvent::User)
        {
                QuitEvent *msg=(QuitEvent*)event;
		QString reason=msg->msg();
		onQuit(reason);
	}
}


void GluxiBot::onQuit(const QString& reason)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		plugin->onQuit(reason);
	}
	myGloox->setPresence(gloox::PresenceUnavailable, reason, 0);
	myGloox->disconnect();
}

QString GluxiBot::getMyNick(gloox::Stanza* s)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		QString res=plugin->resolveMyNick(s);
		if (!res.isEmpty())
			return res;
	}

	return QString::fromStdString(client()->jid().username());
}

AbstractConfigurator* GluxiBot::getConfigurator(gloox::Stanza* s)
{
	QListIterator<PluginRef> it(myPlugins);
	BasePlugin *plugin;
	while (it.hasNext())
	{
		plugin=it.next();
		assert(plugin);
		AbstractConfigurator* cfg=plugin->getConfigurator(s);
		if (cfg)
			return cfg;
	}
	return 0l;
}

BasePlugin* GluxiBot::pluginByStanzaId(gloox::Stanza* s)
{
	QString id=QString::fromStdString(s->findAttribute("id"));
	if (id.isEmpty())
		return 0;
	AsyncRequest* req=myAsyncRequests->byStanzaId(id);
	if (!req) return 0;
	return req->plugin();
}

int GluxiBot::getPriority()
{
	return DataStorage::instance()->getInt("account/priority");
}

void GluxiBot::registerIqHandler(const QString& service)
{
	myGloox->addIqHandler(service);
	//rootDiscoHandler_->rootHandler()->addInfoItem(new FeatureItem(service));
}
