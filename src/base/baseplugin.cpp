#include "baseplugin.h"
#include "gluxibot.h"
#include "glooxwrapper.h"
#include "datastorage.h"
#include "rolelist.h"
#include "messageparser.h"

#include <QString>
#include <QtDebug>

#include <string>
#include <iostream>

BasePlugin::BasePlugin(GluxiBot* parent)
		: QObject(parent)
{
	commands << "HELP" << "LIST";
	pluginId=0;
	priority_=100;
	myShouldIgnoreError=false;
}


BasePlugin::~BasePlugin()
{}

GluxiBot* BasePlugin::bot()
{
	return qobject_cast<GluxiBot*>(parent());
}

void BasePlugin::onConnect()
{
}

void BasePlugin::onDisconnect()
{
}

void BasePlugin::onPresence(gloox::Stanza* /* s */)
{
}

bool BasePlugin::onMessage(gloox::Stanza* s )
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();

	if (BasePlugin::canHandleMessage(s))
	{
		if (cmd.isEmpty())
		{
			reply(s,QString("Plugin: \"%1\" (%2). Use \"!%3 list\" to get command list")
			      .arg(name()).arg(description()).arg(lprefix()));
			return true;
		}
		if (cmd=="HELP")
		{
			if (help().isEmpty())
				reply(s,"No help available");
			else
				reply(s,help());
			return true;
		}

		if (cmd=="LIST")
		{
			if (!commands.count())
				reply(s,QString("Plugin \"%1\" has no commands").arg(lprefix()));
			else
				reply(s,QString("Plugin \"%1\" has followed commands: %2").arg(lprefix()).arg(commands.join(", ").toLower()));
			return true;
		}
	}
	bool res=parseMessage(s);
	if (!res && !shouldIgnoreError())
	{
		if (lprefix().isEmpty())
			reply(s, "Can't handle command. Try \"!list\"");
		else
			reply(s, QString("Plugin \"%1\" can't handle command \"%2\". Try \"!%1 list\"").arg(lprefix()).arg(cmd.toLower()));
	}
	return res;
}

bool BasePlugin::onVCard(const VCardWrapper& vcard)
{
	return false;
}

bool BasePlugin::shouldIgnoreError()
{
	bool t=myShouldIgnoreError;
	myShouldIgnoreError=false;
	return t;
}

bool BasePlugin::parseMessage(gloox::Stanza* /* s */)
{
	return false;
}

bool BasePlugin::canHandlePresence(gloox::Stanza* /* s */)
{
	return false;
}

bool BasePlugin::canHandleIq(gloox::Stanza*)
{
	return false;
}

bool BasePlugin::onIq(gloox::Stanza*)
{
	return false;
}

bool BasePlugin::isMyMessage(gloox::Stanza*)
{
	return false;
}

bool BasePlugin::canHandleMessage(gloox::Stanza* s)
{
	if (isOfflineMessage(s))
		return false;
	if (allMessages())
		return true;
	return (MessageParser::isMessageAcceptable(s, bot()->getMyNick(s), prefix()));
}

QString BasePlugin::getMyNick(gloox::Stanza* s)
{
	return bot()->getMyNick(s);
}

QString BasePlugin::resolveMyNick(gloox::Stanza* s)
{
	return QString::null;
}

bool BasePlugin::isGroupChat(gloox::Stanza* s)
{
	return QString::fromStdString(s->findAttribute("type"))=="groupchat";
}

void BasePlugin::reply(gloox::Stanza* to, const QString& body, bool forcePrivate, bool quoteNick)
{
	QString msg;
	std::string dest;
	if (isGroupChat(to) && !forcePrivate)
	{
		QString bodyToSend=body;
		int maxlength=DataStorage::instance()->getInt("muc/maxmsglength");
		if (bodyToSend.length()>maxlength)
		{
			bodyToSend=bodyToSend.left(maxlength)+"[...]";
		}
		int maxmsglines=DataStorage::instance()->getInt("muc/maxmsglines");
		if (bodyToSend.count('\n')>maxmsglines)
		{
			bodyToSend=bodyToSend.section('\n',0,maxmsglines-1)+"[...]";
		}
		dest=to->from().bare();
		if (quoteNick)
			msg=QString::fromStdString(to->from().resource())+": "+bodyToSend;
		else
			msg=bodyToSend;
	}
	else
	{
		dest=to->from().full();
		msg=body;
		if (msg.length()>10000) //TODO: Divide message to parts
		{
			msg=msg.left(10000)+"[...]";
		}
	}


	gloox::Stanza *st=gloox::Stanza::createMessageStanza(gloox::JID(dest), msg.toStdString());

	st->addAttribute("type",isGroupChat(to) && !forcePrivate ? "groupchat" : "chat");
	bot()->client()->send(st);
}

bool BasePlugin::isOfflineMessage(gloox::Stanza *s)
{
	return (s->hasChild("x","xmlns","jabber:x:delay"));
}

bool BasePlugin::isFromBotOwner(gloox::Stanza *s, bool message)
{
	bool res=getRole(s)>=ROLE_BOTOWNER;
	if (!res && message)
	{
		reply(s,"Only bot owner can do this");
	}
	return res;
}

int BasePlugin::getRole(gloox::Stanza *s)
{
	QString jid1=QString::fromStdString(s->from().full());
	QString jid2=jid1.section('/',0,0);
	int role1=bot()->roles()->get(jid1);
	int role2=bot()->roles()->get(jid2);
	return (role1>role2) ? role1: role2;
}

StorageKey BasePlugin::getStorage(gloox::Stanza*s)
{
	return StorageKey();
}

QString BasePlugin::getNick(gloox::Stanza*s)
{
	if (isGroupChat(s))
	{
		return QString::fromStdString(s->from().resource());
	}
	else
	{
		return QString::fromStdString(s->from().bare()).section('@',0,0)+"@";
	}
}

QString BasePlugin::getJID(gloox::Stanza*s, const QString& nick, bool realJid)
{
	return QString::null;
}

QString BasePlugin::getBotJID(gloox::Stanza* s)
{
	return QString::null;
}

QString BasePlugin::JIDtoNick(const QString& /* jid*/)
{
	return QString::null;
}

void BasePlugin::onQuit(const QString&)
{
}

QString BasePlugin::getPresence(const gloox::Presence& pr)
{
        switch (pr)
        {
        case gloox::PresenceAvailable : return "Available";
        case gloox::PresenceAway: return "Away";
        case gloox::PresenceChat: return "Chat";
        case gloox::PresenceDnd: return "Dnd";
        case gloox::PresenceUnavailable: return "Unavailable";
        case gloox::PresenceXa: return "Xa";
        case gloox::PresenceUnknown: return "Unknown";
        }
        return "Unknown";
}

AbstractConfigurator* BasePlugin::getConfigurator(gloox::Stanza* s)
{
	return 0;
}
