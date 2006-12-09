#include "netplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/common.h"
#include "pingrequest.h"
#include "tracerouterequest.h"
#include "wwwrequest.h"
#include "xeprequest.h"

#include <QtDebug>
#include <QRegExp>

NetPlugin::NetPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "PING" << "TRACEROUTE" << "WWW" << "XEP";
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
		if (!isSafeArg(arg))
		{
			reply(s,"Incorrect character in domain name");
			return true;
		}
		PingRequest *req=new PingRequest(this, s->clone(), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}

	if (cmd=="TRACEROUTE")
        {
                if (arg.isEmpty())
                {
                        reply(s,"Usage: net traceroute [host]");
                        return true;
                }
                if (!isSafeArg(arg))
                {
                        reply(s,"Incorrect character in domain name");
                        return true;
                }
                TraceRouteRequest *req=new TraceRouteRequest(this, s->clone(), arg);
                bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
                req->exec();
                return true;
        }

	if (cmd=="WWW")
	{
		if (arg.isEmpty())
		{
			reply(s, "Usage: net www [URL]");
			return true;
		}
		WWWRequest *req=new WWWRequest(this, s->clone(), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}
	if (cmd=="XEP")
	{
		XepRequest *req=new XepRequest(this, s->clone(), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}
	return false;
}

