#include "adminplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/asyncrequest.h"
#include "base/rolelist.h"

#include <gloox/client.h>
#include <gloox/stanza.h>

AdminPlugin::AdminPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "QUIT" << "ROLES" << "ASYNCCOUNT" << "ASYNCLIST";
}


AdminPlugin::~AdminPlugin()
{}

bool AdminPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();

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
						stanza=QString::fromStdString(req->stanza()->body());
					res+=QString("\n%1: %2").arg(plugin).arg(stanza);
				}
				reply(s,"Active async requests: "+res);
			}
		}
		else
			reply(s,"Only owner can do this");
		return true;
	}

	return false;
}
