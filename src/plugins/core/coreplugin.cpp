#include "coreplugin.h"
#include "base/gluxibot.h"
#include "base/pluginlist.h"
#include "base/messageparser.h"

#include <QtDebug>

CorePlugin::CorePlugin(GluxiBot *parent)
		: BasePlugin(parent)
{}


CorePlugin::~CorePlugin()
{}

bool CorePlugin::onMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	qDebug() << parser.joinBody();
	QString cmd=parser.nextToken().toUpper();

	if (cmd=="HELP")
	{
		reply(s,QString("libGLOOX Jabber Bot (http://gluxi.inhex.net). Type \"!list\" to get plugin list\nType \"!<plugin> list\" to get plugin command list"));
		return true;
	}
	if (cmd=="LIST")
	{
		QStringList plg;
		PluginList *plgList=bot()->plugins();
		QString pr;
		for (int i=0; i< plgList->count(); i++)
		{
			pr=plgList->at(i)->prefix().toLower();
			if (!pr.isEmpty())
				plg << pr;
		}
		qSort(plg.begin(), plg.end());
		reply(s,QString("Available plugins: %1").arg(plg.join(", ")));
		return true;
	}
	return false;
}
