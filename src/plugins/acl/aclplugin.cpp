#include "aclplugin.h"
#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/rolelist.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QTime>

AclPlugin::AclPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	priority_=10; // We should be able to drop stanzas
}

AclPlugin::~AclPlugin()
{
}

bool AclPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	return false;
}
