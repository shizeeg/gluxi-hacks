#include "miscplugin.h"
#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/rolelist.h"
#include "base/datastorage.h"
#include "base/asyncrequestlist.h"
#include "base/common.h"

#include "bcrequest.h"

#include <QtDebug>
#include <QTime>

MiscPlugin::MiscPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "TEST" << "DATE" << "TIME" << "SAY" << "BC" << "DECIDE" << "FLIP";

	sayJidDisabled_=DataStorage::instance()->getInt("cmd/disable_misc_sayjid");
	if (!sayJidDisabled_)
	{
		commands << "SAYJID";
	}
}

MiscPlugin::~MiscPlugin()
{
}

bool MiscPlugin::parseMessage(gloox::Stanza* s, const QStringList& flags)
{
	Q_UNUSED(flags);

	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	qDebug() << "Got CMD: " << cmd << "; length=" << cmd.length();

	if (cmd=="TEST")
	{
		reply(s, "passed");
		return true;
	}
	if (cmd=="DATE")
	{
		reply(s, QDate::currentDate().toString(Qt::LocaleDate));
		return true;
	}
	if (cmd=="TIME")
	{
		reply(s, QTime::currentTime().toString(Qt::LocaleDate));
		return true;
	}
	if (!sayJidDisabled_ && cmd=="SAYJID")
	{
		if (getRole(s)<ROLE_ADMIN)
		{
			reply(s,"You should be moderator to do this");
			return true;
		}
		QString dst=parser.nextToken();
		if (dst.isEmpty())
		{
			reply(s,"No dest. JID specified");
			return true;
		}
		QString body=parser.joinBody();
		gloox::Stanza* out=gloox::Stanza::createMessageStanza(gloox::JID(dst.toStdString()),body.toStdString());
		bot()->client()->send(out);
		return true;
	}
	if (cmd=="SAY")
	{
		if (getRole(s)<ROLE_MODERATOR)
		{
			reply(s,"You should be moderator to do this");
			return true;
		}
		QString body=parser.joinBody();
		reply(s,body,false, false);
		return true;
	}
	if (cmd=="BC")
	{
		BcRequest *req = new BcRequest(this, new gloox::Stanza(s), parser);
		bot()->asyncRequests()->append(req);
		req->exec();
		return true;
	}
	if (cmd=="DECIDE")
	{
		QStringList choices = parser.getTokens();
		if (choices.count() <= 3)
		{
			reply(s, "You have no choice.");
			return true;
		}
		srand( time(NULL) );
		int r = random() % (choices.count()-2); // strip misc and decide tokens
		reply(s, choices[r+2]);
		return true;
	}
	if (cmd=="TURN")
	{
		QString body = parser.joinBody();
		if (body.isEmpty())
		{
			reply(s, "!misc turn <text>");
			return true;
		}
		reply(s, turn(body));
		return true;
	}
	if (cmd == "FLIP")
	{
		QString body = parser.joinBody().toLower();
		if (body.isEmpty())
		{
			reply(s, "!misc flip <text>");
			return true;
		}

		QString msg;
		for (int i = body.count() - 1; i > -1; --i)
		{
			msg += flipChar(body[i]);
		}

		reply(s, msg);
		return true;
	}
	return false;
}
