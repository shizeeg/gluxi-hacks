#include "aliasplugin.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/rolelist.h"

#include <QList>
#include <QtDebug>

#include <assert.h>

AliasPlugin::AliasPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "SHOW"  << "CLEAR" << "COUNT" << "ADD" << "DEL";
}


AliasPlugin::~AliasPlugin()
{}

bool AliasPlugin::canHandleMessage(gloox::Stanza* s)
{
	if (isOfflineMessage(s))
		return false;
	if (BasePlugin::canHandleMessage(s))
		return true;
	if (bot()->isMyMessage(s))
		return false;
	if (s->hasAttribute("glooxbot_alias"))
		return false;
	QString body=getBody(s,false);
	qDebug() << "AliasPlugin::canHandleMessage() " << body;
	return (!body.isEmpty());
}

bool AliasPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s,false);
	qDebug() << "AliasPlugin::parseMessage() " << body;
	QString cmd=body.section(' ',0,0).toUpper();
	QString arg=body.section(' ',1);
	// 	reply(s,QString("got: \"%1\"").arg(body));
	if (cmd=="ALIAS")
		return parseCommands(s);

	QString res=aliases.get(bot()->getStorage(s),cmd);

	if (!res.isEmpty())
	{
		QString expanded=expandAlias(res,arg);
		gloox::Tag *tg=s->findChild("body");
		assert(tg);
		tg->setCData(expanded.toStdString());
		s->addAttribute("glooxbot_alias","true");
		s->finalize();
		
		// TODO: Fix alias plugin
		bot()->client()->handleMessage(s);
		return true;
	}
	myShouldIgnoreError=true;
	return false;
}

bool AliasPlugin::parseCommands(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper(); // First arg - "ALIAS"
	QString arg=body.section(' ',1);

	// 	reply(s,"Command: "+cmd);

	if (cmd.isEmpty() || cmd=="LIST" || cmd=="HELP")
		return BasePlugin::onMessage(s);

/*	if (!isFromOwner(s) &&
	        bot()->tmpOwners()->indexOf(QString::fromStdString(s->from().full()))<0)*/
	if (getRole(s) < ROLE_ADMIN)
	{
		reply(s,"You should be admin to edit aliases");
		return true;
	}

	if (cmd=="SHOW")
	{
		QString res;
		QMap<QString, QString> all=aliases.getAll(bot()->getStorage(s));
		int cnt=all.count();
		if (!cnt)
		{
			reply(s,"Alias list is empty");
			return true;
		}
		for (int i=0; i<cnt; i++)
			res+=QString("\n%1) %2=%3").arg(i+1).arg(all.keys()[i].toLower()).arg(all.values()[i]);
		reply(s,QString("Aliases:%1").arg(res));
		return true;
	}
	if (cmd=="CLEAR")
	{
		aliases.clear(bot()->getStorage(s));
		reply(s,"Cleared");
		return true;
	}
	if (cmd=="COUNT")
	{
		reply(s,QString("Currntly I have %1 aliases").arg(aliases.count(bot()->getStorage(s))));
		return true;
	}
	//arg=arg.toUpper();
	QString name=arg.section('=',0,0).toUpper().trimmed();
	QString value=arg.section('=',1).trimmed();
	if (cmd=="ADD")
	{
		if (name.isEmpty() || value.isEmpty())
		{
			reply(s,"Syntax: !alias add <name>=<value>, where value can contain %1, %2.. - arguments; %* - all arguemnts");
			return true;
		}
		/*		QRegExp valExp("[^\\\\]%[0-9]");
				if (value.startsWith("%") || valExp.indexIn(value)>=0)
				{
					reply(s, "value shouldn't conatin \"%\"");
					return true;
				}*/

		int res=aliases.append( bot()->getStorage(s),name,value);
		switch (res)
		{
		case 0: reply(s, "Can't store"); break;
		case 1: reply(s, "Saved"); break;
		case 2: reply(s, "Updated"); break;
		}

		return true;
	}
	if (cmd=="DEL")
	{
		if (aliases.remove(bot()->getStorage(s),name))
		{
			reply(s,"Deleted");
		}
		else
		{
			reply(s,"Can't delete");
		}
		return true;
	}
	return false;
}

QString AliasPlugin::expandAlias(const QString&alias, const QString& args)
{
	QString res=alias;
	QRegExp exp;
	exp.setMinimal(false);
	exp.setCaseSensitivity(Qt::CaseInsensitive);
	int idx=1;
	while (1)
	{
		bool wasRepl=false;
		exp.setPattern(QString("[^\\\\]\\%")+QString::number(idx));
		while (1)
		{
			int ps=exp.indexIn(res);
			if (ps<0) break;
			res.remove(ps+1,exp.matchedLength()-1);
			res.insert(ps+1, args.section(' ',idx-1,idx-1));
			wasRepl=1;
		}
		if (!wasRepl) break;
		idx++;
	}
	exp.setPattern(QString("[^\\\\]\\%\\*"));
	while (1)
	{
		int ps=exp.indexIn(res);
		if (ps<0) break;
		res.remove(ps+1,exp.matchedLength()-1);
		res.insert(ps+1, args.section(' ',idx-1));
	}
	return res.trimmed();
}
