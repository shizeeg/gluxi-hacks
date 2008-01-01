#include "miscplugin.h"
#include "base/messageparser.h"

#include <QtDebug>
#include <QTime>

MiscPlugin::MiscPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "TEST" << "DATE" << "TIME";
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
	return false;
}
