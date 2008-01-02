#include "aliasplugin.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/rolelist.h"
#include "base/messageparser.h"

#include <QList>
#include <QtDebug>

#include <assert.h>

AliasPlugin::AliasPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "SHOW" << "CLEAR" << "COUNT" << "ADD" << "DEL";
}

AliasPlugin::~AliasPlugin()
{
}

bool AliasPlugin::canHandleMessage(gloox::Stanza* s)
{
	if (isOfflineMessage(s))
		return false;
	if (BasePlugin::canHandleMessage(s))
		return true;
	if (bot()->isMyMessage(s))
		return false;

	//Stanza with already expanded alias
	if (s->hasAttribute("glooxbot_alias"))
		return false;
	return MessageParser(s,getMyNick(s)).isForMe();
}

bool AliasPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));

	QString cmd=parser.nextToken().toUpper();
	if (cmd=="ALIAS")
		return parseCommands(s, parser);

	parser.back();
	QString res=aliases.get(bot()->getStorage(s), cmd);
	QString firstArg=res.section(' ',0,0).toUpper();

	bool noWrap=false;
	if (firstArg=="/NOWRAP")
	{
		noWrap=true;
		res=res.section(' ', 1);
	}

	if (!res.isEmpty())
	{
		//		QString expanded=expandAlias(res,arg);
		while (1)
		{
			QString originalItem=res.section(";",0,0).trimmed();
			if (!noWrap)
				originalItem.replace(' ', '\n');

			if (originalItem.isEmpty())
				break;

			res=res.section(";",1).trimmed();
			QString item=expandAlias(originalItem, parser);

			//qDebug() << "------ Alias:\n| original: " << originalItem
			//<< "\n| expanded: " << item;

			if (item.isEmpty())
				continue;
			gloox::Tag *tg=s->findChild("body");
			assert(tg);
			tg->setCData(item.toStdString());
			s->addAttribute("glooxbot_alias", "true");
			s->finalize();
			bot()->client()->handleMessage(s, 0);
		}
		return true;
	}
	myShouldIgnoreError=true;
	return false;
}

bool AliasPlugin::parseCommands(gloox::Stanza* s, MessageParser& parser)
{
	QString cmd=parser.nextToken().toUpper();

	if (cmd.isEmpty() || cmd=="LIST" || cmd=="HELP")
		return false; //BasePlugin::onMessage(s);

	if (getRole(s) < ROLE_ADMIN)
	{
		reply(s, "You should be admin to edit aliases");
		return true;
	}

	if (cmd=="SHOW")
	{
		QString res;
		QMap<QString, QString> all=aliases.getAll(bot()->getStorage(s));
		int cnt=all.count();
		if (!cnt)
		{
			reply(s, "Alias list is empty");
			return true;
		}
		for (int i=0; i<cnt; i++)
			res+=QString("\n%1) %2=%3").arg(i+1).arg(all.keys()[i].toLower()).arg(all.values()[i]);
		reply(s, QString("Aliases:%1").arg(res));
		return true;
	}
	if (cmd=="CLEAR")
	{
		aliases.clear(bot()->getStorage(s));
		reply(s, "Cleared");
		return true;
	}
	if (cmd=="COUNT")
	{
		reply(s, QString("Currntly I have %1 aliases").arg(aliases.count(bot()->getStorage(s))));
		return true;
	}
	//arg=arg.toUpper();
	QString arg=parser.joinBody();
	QString name=arg.section('=',0,0).toUpper().trimmed();
	QString value=arg.section('=',1).trimmed();
	if (cmd=="ADD")
	{
		if (name.isEmpty() || value.isEmpty())
		{
			reply(s, "Syntax: !alias add <name>=<value>,"
				" where value can contain"
				" %1, %2.. - arguments; %* - all arguemnts");
			return true;
		}
		int res=aliases.append(bot()->getStorage(s), name, value);
		switch (res)
		{
		case 0:
			reply(s, "Can't store");
			break;
		case 1:
			reply(s, "Saved");
			break;
		case 2:
			reply(s, "Updated");
			break;
		}

		return true;
	}
	if (cmd=="DEL")
	{
		if (aliases.remove(bot()->getStorage(s), name))
		{
			reply(s, "Deleted");
		}
		else
		{
			reply(s, "Can't delete");
		}
		return true;
	}
	return false;
}

QString AliasPlugin::expandAlias(const QString&alias, MessageParser parser)
{
	QString res=alias;
	QRegExp exp;
	exp.setMinimal(false);
	exp.setCaseSensitivity(Qt::CaseInsensitive);
	int idx=1;
	while (1)
	{
		bool wasRepl=false;
		int offset=0;
		exp.setPattern(QString("[^\\\\]\\%")+QString::number(idx));
		QString subStr=parser.nextToken();
		while (1)
		{
			int ps=exp.indexIn(res, offset);
			if (ps<0)
				break;
			res.remove(ps+1, exp.matchedLength()-1);
			res.insert(ps+1, subStr);
			wasRepl=1;
			offset=ps+subStr.length();
		}
		if (!wasRepl)
			break;
		idx++;
	}
	exp.setPattern(QString("[^\\\\]\\%\\*"));
	int offset=0;
	QString subStr=parser.joinBody();
	while (1)
	{
		int ps=exp.indexIn(res, offset);
		if (ps<0)
			break;
		res.remove(ps+1, exp.matchedLength()-1);
		res.insert(ps+1, subStr);
		offset=ps+subStr.length();
	}
	return res.trimmed();
}
