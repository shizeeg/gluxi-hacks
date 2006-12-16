#include "adminplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/asyncrequest.h"

#include <gloox/client.h>
#include <gloox/stanza.h>

AdminPlugin::AdminPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "QUIT" << "TMPOWNERS" << "ASYNCCOUNT" << "ASYNCLIST";
}


AdminPlugin::~AdminPlugin()
{}

bool AdminPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();

	if (cmd=="QUIT")
	{
		if (isFromOwner(s))
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
	if (cmd=="TMPOWNERS")
	{
		if (isFromOwner(s))
		{
			reply(s,QString("Temporary bot owners: \n%1").arg(bot()->tmpOwners()->join("\n")));
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
		if (isFromOwner(s))
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
