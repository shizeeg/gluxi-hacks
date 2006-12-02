#include "netplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "pingrequest.h"

#include <QtDebug>
#include <QRegExp>

NetPlugin::NetPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "PING";
}


NetPlugin::~NetPlugin()
{}

bool NetPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();
	QString arg=body.section(' ',1);
	qDebug() << "Got CMD: " << cmd << "; length=" << cmd.length();

	if (cmd=="PING")
	{
		if (arg.isEmpty())
		{
			reply(s,"Usage: net ping [host]");
			return true;
		}
		QRegExp exp("^[0-9A-Za-z_-\\.]*$");
		exp.setMinimal(false);
		if (!exp.exactMatch(arg))
		{
			reply(s,"Incorrect character in domain name");
			return true;
		}
		PingRequest *req=new PingRequest(this, s->clone(), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}
	return false;
}

