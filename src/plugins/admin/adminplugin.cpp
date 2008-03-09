#include "adminplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/asyncrequest.h"
#include "base/rolelist.h"
#include "base/messageparser.h"
#include "base/glooxwrapper.h"

#include <gloox/client.h>
#include <gloox/stanza.h>

#include <QtDebug>

AdminPlugin::AdminPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "QUIT" << "ROLES" << "ASYNCCOUNT" << "ASYNCLIST" << "PRESENCE";
}


AdminPlugin::~AdminPlugin()
{}

bool AdminPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s);
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();

	if (cmd=="QUIT")
	{
		if (isFromBotOwner(s))
		{
			reply(s,"Ok");
			
			bot()->onQuit("QUIT command from bot owner");
		}
		else
		{
			reply(s,"Only owner can do this");
		}
		return true;
	}
	if (cmd=="ROLES")
	{
		if (isFromBotOwner(s))
		{
			QString res;
			RoleList *list=bot()->roles();
			int cnt=list->keys().count();
			for (int i=0; i<cnt; i++)
			{
				res+=QString("\n%1: %2").arg(list->keys()[i]).arg(list->get(list->keys()[i]));
			}
			reply(s,QString("Roles: %1").arg(res));
		}
		else
		{
			reply(s,"Only owner can do this");
		}
		return true;
	}
	
	if (cmd=="ASYNCCOUNT")
	{
		reply(s,QString("Async requests count: %1").arg(bot()->asyncRequests()->count()));
		return true;
	}

	if (cmd=="ASYNCLIST")
	{
		if (isFromBotOwner(s))
		{
			int cnt=bot()->asyncRequests()->count();
			if (cnt==0)
				reply(s,"No async requests found");
			else
			{
				QString res;
				for (int i=0; i<cnt; i++)
				{
					AsyncRequest *req=bot()->asyncRequests()->at(i);
					QString plugin;
					QString stanza;
					if (req->plugin())
						plugin=req->plugin()->name();
					else
						plugin="UNKNOWN";
					if (req->stanza())
					{
						stanza=QString::fromStdString(req->stanza()->body());
						stanza.replace('\n',' ');
					}
					res+=QString("\n%1: %2").arg(plugin).arg(stanza);
				}
				reply(s,"Active async requests: "+res);
			}
		}
		else
			reply(s,"Only owner can do this");
		return true;
	}
	
	if (cmd=="PRESENCE")
	{
		if (!isFromBotOwner(s))
		{
			reply(s,"Only owner can do this");
			return true;
		}
		QString pr=parser.nextToken().toUpper();
		gloox::Presence presence;
		if (pr=="AVAILABLE")
			presence=gloox::PresenceAvailable;
		else if (pr=="AWAY")
			presence=gloox::PresenceAway;
		else if (pr=="XA")
			presence=gloox::PresenceXa;
		else if (pr=="DND")
			presence=gloox::PresenceDnd;
		else if (pr=="CHAT")
			presence=gloox::PresenceChat;
		else
		{
			reply(s,"Available presences: available, away, xa, dnd, chat");
			return true;
		}
		QString status=parser.nextToken();
		bot()->client()->setPresence(presence, status, bot()->getPriority());
		return true;
	}

	return false;
}
