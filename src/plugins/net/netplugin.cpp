#include "netplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/common.h"
#include "pingrequest.h"
#include "tracerouterequest.h"
#include "wwwrequest.h"
#include "xeprequest.h"
#include "googlerequest.h"
#include "svnrequest.h"
#include "base/messageparser.h"

#include <QtDebug>
#include <QRegExp>

NetPlugin::NetPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "PING" << "TRACEROUTE" << "WWW" << "XEP" << "GOOGLE" << "SVN" << "HEADERS";
}


NetPlugin::~NetPlugin()
{}

bool NetPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.joinBody();
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
		PingRequest *req=new PingRequest(this, new gloox::Stanza(s), arg);
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
                TraceRouteRequest *req=new TraceRouteRequest(this, new gloox::Stanza(s), arg);
                bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
                req->exec();
                return true;
        }

	if (cmd=="WWW" || cmd=="HEADERS")
	{
		if (arg.isEmpty())
		{
			reply(s, "Usage: net www [URL]");
			return true;
		}
		WWWRequest *req=new WWWRequest(this, new gloox::Stanza(s), cmd, arg);
		bot()->asyncRequests()->append(req);
		req->launch();
		return true;
	}
	
	if (cmd=="SVN")
	{
		if (arg.isEmpty() || !isSafeArg(arg))
		{
			reply(s, "Usage: net svn [repository URL] "+arg);
			return true;
		}
		SVNRequest *req=new SVNRequest(this, new gloox::Stanza(s), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}

	if (cmd=="XEP")
	{
		XepRequest *req=new XepRequest(this, new gloox::Stanza(s), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}
	if (cmd=="GOOGLE")
	{
		GoogleRequest *req=new GoogleRequest(this, new gloox::Stanza(s), arg);
		bot()->asyncRequests()->append(qobject_cast<AsyncRequest*>(req));
		req->exec();
		return true;
	}

	return false;
}

