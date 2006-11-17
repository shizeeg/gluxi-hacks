#include "wordplugin.h"
#include "base/gluxibot.h"

#include <QtDebug>
#include <QTime>

WordPlugin::WordPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "ADD" << "COUNT" << "CLEAR" << "SHOW" << "SHOWPRIV" << "DEL";
}


WordPlugin::~WordPlugin()
{}

bool WordPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();
	QString arg=body.section(' ',1);

	if (cmd=="ADD")
	{
		QString name=arg.section('=',0,0);
		QString value=arg.section('=',1);
		if (name.isEmpty() || value.isEmpty())
		{
			reply(s, "Definition should be like name=value");
			return true;
		}
		int res=words.append(bot()->getStorage(s),name,getNick(s),value);
		if (res==1)
			reply(s,"Saved");
		else
			if (res==2)
				reply(s,"Updated");
			else
				reply(s,"Can't save");
		return true;
	}
	if (cmd=="COUNT")
	{
		reply(s,QString("Currently I know %1 words").arg(words.count(bot()->getStorage(s))));
		return true;
	}
	if (cmd=="CLEAR")
	{
		words.clear(bot()->getStorage(s));
		reply(s,"Ok");
		return true;
	}
	if (cmd=="SHOW")
	{
		QString nick;
		QString value=words.get(bot()->getStorage(s),arg,&nick);
		if (value.isEmpty())
			reply(s,"I don't know");
		else
		{
			QString toSay=QString("%1 says that %2 = %3").arg(nick).arg(arg).arg(value);
			qDebug() << toSay;
			reply(s, toSay);	
		}
		return true;
	}
	if (cmd=="SHOWPRIV")
	{
		//TODO: Implement nick handler
		QString dest=arg.section(' ',0,0);
		QString word=arg.section(' ',1);
		if (dest.isEmpty() || word.isEmpty())
		{
			reply(s,"Syntax: SHOWPRIV <NICK> <WORD>");
			return true;
		}
		if (bot()->tmpOwners()->indexOf(QString::fromStdString(s->from().full()))<0)
		{
			reply(s,"You should be at least admin to do this");
			return true;
		}
				
		qDebug() << "DEST=" << dest;
		QString jid=bot()->getJID(s,dest);
		if (jid.isEmpty())
		{
			reply(s,"Nick not found");
			return true;
		}
		QString nick;
		QString value=words.get(bot()->getStorage(s),word,&nick);
		if (value.isEmpty())
		{
			reply(s,"I don't know");
			return true;
		}
		reply(s,"Ok");
		s->addAttribute("from",jid.toStdString());
		s->finalize();
		qDebug() << QString::fromStdString(s->xml());
		QString toSay=QString("%1 says that %2 = %3").arg(nick).arg(word).arg(value);
		reply(s,toSay,true);
		return true;
	}
	if (cmd=="SHOWALL")
	{
		QMap<QString,QString> all=words.getAll(bot()->getStorage(s),arg);	
		int cnt=all.count();
		if (!cnt)
		{
			reply(s,"I don't know");
			return true;
		}
		QString res;
		for (int i=0; i<cnt; i++)
			res+=QString("\n%1) %2 says that %3 - %4")
				.arg(i+1)
				.arg(all.keys()[i])
				.arg(arg)
				.arg(all.values()[i]);
		reply(s,QString("I know that: %1").arg(res));
		return true;
	}
	if (cmd=="DEL")
	{
		int res=words.remove(bot()->getStorage(s),arg);
		if (res)
			reply(s,"Removed");
		else
			reply(s,"Can't remove");

		return true;
	}
	return false;
}

