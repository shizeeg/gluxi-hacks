#include "miscplugin.h"
#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"

#include <QtDebug>
#include <QTime>

MiscPlugin::MiscPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "TEST" << "DATE" << "TIME" << "SAY" << "SAYJID" << "SAYJIDGC"	;
}

MiscPlugin::~MiscPlugin()
{
}

bool MiscPlugin::parseMessage(gloox::Stanza* s)
{
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
	if (cmd=="SAYJID" || cmd=="SAYJIDGC") 
	{
		QString dst=parser.nextToken();
		if (dst.isEmpty())
		{
			reply(s,"No dest. JID specified");
			return true;
		}
		QString body=parser.joinBody();
		gloox::Stanza* out=gloox::Stanza::createMessageStanza(gloox::JID(dst.toStdString()),body.toStdString());
		if (cmd=="SAYJIDGC")
			out->addAttribute("type","groupchat");
		bot()->client()->send(out);
		return true;
	}
	if (cmd=="SAY") 
	{
		QString body=parser.joinBody();
		reply(s,body,false, false);
		return true;
	}
	return false;
}
