#include "wordplugin.h"
#include "base/gluxibot.h"
#include "base/rolelist.h"
#include "base/messageparser.h"

#include <QtDebug>
#include <QTime>

WordPlugin::WordPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "ADD" << "COUNT" << "CLEAR" << "SHOW" << "SHOWPRIV" << "SHOWJID" << "DEL";
}

WordPlugin::~WordPlugin()
{
}

bool WordPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.joinBody();

	if (cmd=="COUNT")
	{
		reply(s, QString("Currently I know %1 words").arg(words.count(bot()->getStorage(s))));
		return true;
	}

	if (cmd=="SHOW")
	{
		QString nick;
		QString value=words.get(bot()->getStorage(s), arg, &nick);
		if (value.isEmpty())
			reply(s, "I don't know");
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
		QString dest=parser.nextToken();
		QString word=parser.nextToken();

		bool allowUser=false;

		if (dest.isEmpty() && !word.isEmpty())
		{
			allowUser=true;
			dest=getNick(s);
		}
		else
		{
			if (word.isEmpty() && !dest.isEmpty())
			{
				allowUser=true;
				word=dest;
				dest=getNick(s);
			}
		}

		if (dest.isEmpty() || word.isEmpty())
		{
			reply(s, "Syntax: SHOWPRIV <NICK> <WORD>");
			return true;
		}
		if (!allowUser && getRole(s)<ROLE_MODERATOR)
		{
			reply(s, "You should be at least admin to do this");
			return true;
		}

		QString jid=bot()->getJID(s, dest);
		if (jid.isEmpty())
		{
			reply(s, "Nick not found: "+dest);
			return true;
		}
		QString nick;
		QString value=words.get(bot()->getStorage(s), word, &nick);
		if (value.isEmpty())
		{
			reply(s, "I don't know");
			return true;
		}
		reply(s, "Ok");
		s->addAttribute("from", jid.toStdString());
		s->finalize();
		qDebug() << QString::fromStdString(s->xml());
		QString toSay=QString("%1 says that %2 = %3").arg(nick).arg(word).arg(value);
		reply(s, toSay, true);
		return true;
	}

	if (cmd=="SHOWJID")
	{
		QString dest=parser.nextToken();
		QString word=parser.nextToken();

		if (dest.isEmpty() || word.isEmpty())
		{
			reply(s, "Syntax: SHOWJID <JID> <WORD>");
			return true;
		}
		if (getRole(s)<ROLE_MODERATOR)
		{
			reply(s, "You should be at least moderator to do this");
			return true;
		}

		QString nick;
		QString value=words.get(bot()->getStorage(s), word, &nick);
		if (value.isEmpty())
		{
			reply(s, "I don't know");
			return true;
		}
		reply(s, "Ok");
		s->addAttribute("from", dest.toStdString());
		s->finalize();
		
		QString toSay=QString("%1 says that %2 = %3").arg(nick).arg(word).arg(value);
		reply(s, toSay, true);
		return true;
	}
	
	if (cmd=="SHOWALL")
	{
		QMap<QString,QString> all=words.getAll(bot()->getStorage(s), arg);
		int cnt=all.count();
		if (!cnt)
		{
			reply(s, "I don't know");
			return true;
		}
		QString res;
		for (int i=0; i<cnt; i++)
			res+=QString("\n%1) %2 says that %3 - %4")
			.arg(i+1)
			.arg(all.keys()[i])
			.arg(arg)
			.arg(all.values()[i]);
		reply(s, QString("I know that: %1").arg(res));
		return true;
	}

	if (cmd=="ADD" || cmd=="DEL" || cmd=="CLEAR")
	{
		if (getRole(s)<ROLE_ADMIN)
		{
			reply(s, "You have no rights to do this");
			return true;
		}

		if (cmd=="ADD")
		{
			//Reparse Stanza with space as separator
			MessageParser newParser(s, getMyNick(s), ' ');
			newParser.nextToken();
			newParser.nextToken(); //cmd
			arg=newParser.joinBody();
			QString name=arg.section('=', 0, 0);
			QString value=arg.section('=', 1);
			if (name.isEmpty() || value.isEmpty())
			{
				reply(s, "Definition should be like name=value");
				return true;
			}
			QString nick=bot()->JIDtoNick(QString::fromStdString(s->from().full()));
			int res=words.append(bot()->getStorage(s), name, nick, value);
			if (res==1)
				reply(s, "Saved");
			else if (res==2)
				reply(s, "Updated");
			else
				reply(s, "Can't save");
			return true;
		}

		if (cmd=="CLEAR")
		{
			words.clear(bot()->getStorage(s));
			reply(s, "Ok");
			return true;
		}

		if (cmd=="DEL")
		{
			int res=words.remove(bot()->getStorage(s), arg);
			if (res)
				reply(s, "Removed");
			else
				reply(s, "Can't remove");

			return true;
		}
	}
	return false;
}

