#include "baseplugin.h"
#include "gluxibot.h"
#include "datastorage.h"

#include <QString>

#include <string>
#include <iostream>

BasePlugin::BasePlugin(GluxiBot* parent)
		: QObject(parent)
{
	commands << "HELP" << "LIST";
	pluginId=0;
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

void BasePlugin::onPresence(gloox::Stanza* /* s */)
{
}

bool BasePlugin::onMessage(gloox::Stanza* s )
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();

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
	return (!getBody(s).isEmpty());
}

QString BasePlugin::getBody(gloox::Stanza* s, bool usePrefix)
{
	if (isOfflineMessage(s))
		return QString::null;

	QString body=QString::fromStdString(s->body());
	int isec=0;
	QString first=body.section(' ',isec,isec).toUpper();
	QString nick=QString::fromStdString(bot()->client()->jid().username()).toUpper()+":";
	bool isMe=false;
	if (first.startsWith('!'))
	{
		isMe=true;
		first.remove(0,1);
	}
	if (first==nick)
	{
		isMe=true;
		isec=1;
		first=body.section(' ',isec,isec).toUpper();
	}

	if (!isMe && isGroupChat(s))
		return QString::null;

	QString myPrefix;
	if (usePrefix)
		myPrefix=prefix();

	if (myPrefix.isEmpty())
	{
		return QString(first+" "+body.section(' ',isec+1)).trimmed();
	}

	if (first==myPrefix)
	{
		first=body.section(' ',isec+1);
		if (first.isEmpty()) first=" ";
		return first;
	}
	return QString::null;
}

bool BasePlugin::isGroupChat(gloox::Stanza* s)
{
	return QString::fromStdString(s->findAttribute("type"))=="groupchat";
}

void BasePlugin::reply(gloox::Stanza* to, const QString& body, bool forcePrivate)
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
		msg=QString::fromStdString(to->from().resource())+": "+bodyToSend;
	}
	else
	{
		dest=to->from().full();
		msg=body;
	}


	gloox::Stanza *st=gloox::Stanza::createMessageStanza(gloox::JID(dest), msg.toStdString());

	st->addAttribute("type",isGroupChat(to) && !forcePrivate ? "groupchat" : "chat");

	std::cout << "-----------------" << std::endl << st->xml() << std::endl << std::endl;
	bot()->client()->send(st);
}

bool BasePlugin::isOfflineMessage(gloox::Stanza *s)
{
	return (s->hasChild("x","xmlns","jabber:x:delay"));
}

bool BasePlugin::isFromOwner(gloox::Stanza *s, bool message)
{
	QString jid=QString::fromStdString(s->from().bare());
	bool res=bot()->owners()->indexOf(jid)>=0;
	if (!res && message)
	{
		reply(s,"Only bot owner can do this");
	}
	return res;
}

int BasePlugin::getStorage(gloox::Stanza*s)
{
	return 0;
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

QString BasePlugin::getJID(gloox::Stanza*s, const QString& nick)
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

