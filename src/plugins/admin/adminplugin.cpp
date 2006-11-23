#include "adminplugin.h"
#include "base/gluxibot.h"

#include <gloox/client.h>

AdminPlugin::AdminPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "QUIT";
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

	return false;
}
