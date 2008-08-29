#include "aclplugin.h"
#include "acllist.h"
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
	aclList_=new AclList();
	aclMap_=aclList_->getAll();
	commands << "SHOW" << "ADD" << "DEL" << "CLEAR";
}

AclPlugin::~AclPlugin()
{
	delete aclList_;
}

bool AclPlugin::canHandleMessage(gloox::Stanza* s)
{
	if (isOfflineMessage(s))
		return false;
	if (BasePlugin::canHandleMessage(s))
		return true;
	if (bot()->isMyMessage(s))
		return false;
	return true;
}

bool AclPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	QString cmd=parser.nextToken().toUpper();
	if (cmd==name())
		return parseCommands(s, parser);
	myShouldIgnoreError=1;

	QString fromJid=QString::fromStdString(s->from().full());
	if (!isJidAccepted(fromJid))
		return true;

	fromJid=bot()->getJID(s, QString::null, true);
	if (!isJidAccepted(fromJid))
		return true;
	return false;
}

bool AclPlugin::parseCommands(gloox::Stanza* s, MessageParser& parser)
{
	if (getRole(s) < ROLE_BOTOWNER)
	{
		reply(s, "You should be bot-owner to do this");
		return true;
	}

	QString cmd=parser.nextToken().toUpper();
	if (cmd=="SHOW")
	{
		QString res;
		QList<QString> keys=aclMap_.keys();
		for (QList<QString>::iterator it=keys.begin(); it!=keys.end(); ++it)
		{
			QString key=*it;
			res+="\n"+key+": "+aclMap_.value(key);
		}
		reply(s, "Access list: "+res);
		return true;
	}

	if (cmd=="ADD")
	{
		QString name=parser.nextToken().toLower();
		QString value=parser.nextToken().toLower();
		bool ok=true;
		value.toInt(&ok);
		if (name.isEmpty() || value.isEmpty() || !ok)
		{
			reply(s,"Incorrect syntax");
			return true;
		}
		aclList_->append(name, value);
		reply(s, "Ok");
		aclMap_=aclList_->getAll();
		return true;
	}
	if (cmd=="DEL")
	{
		QString name=parser.nextToken().toLower();
		if (name.isEmpty())
		{
			reply(s, "Incorrect syntax");
			return true;
		}
		if (!aclMap_.contains(name))
		{
			reply(s, "No item found");
			return true;
		}
		aclList_->remove(name);
		reply(s, "Ok");
		aclMap_=aclList_->getAll();
		return true;
	}
	if (cmd=="CLEAR")
	{
		aclList_->clear();
		aclMap_=aclList_->getAll();
		reply(s, "Ok");
		return true;
	}
	return false;
}

int AclPlugin::getAccessLevel(const QString& key)
{
	if (!aclMap_.contains(key))
		return 0;
	return aclMap_.value(key).toInt();
}

bool AclPlugin::isJidAccepted(const QString& jid)
{
	QString fromJid=jid;
	if (getAccessLevel(fromJid)<0)
		return false;
	fromJid=fromJid.section('/',0,-2);
	if (getAccessLevel(fromJid)<0)
		return false;
	return true;
}
