#include "wordplugin.h"
#include "base/gluxibot.h"
#include "base/rolelist.h"
#include "base/messageparser.h"

#include <QtDebug>
#include <QTime>

WordPlugin::WordPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "ADD" << "COUNT" << "NAMES" << "CLEAR" << "SHOW" << "SHOWPRIV" << "SHOWPRIVSILENT" << "SHOWJID" << "DEL";
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

	if (cmd=="NAMES")
	{
		QStringList list=words.getNames(bot()->getStorage(s));
		if (list.isEmpty())
			reply(s,"I don't know any words");
		else
			reply(s, QString("I know followed words: %1").arg(list.join(", ")));
		return true;
	}

	if (cmd=="SHOW")
	{
		QString wrd=parser.nextToken();
		QString nick;
		QString value=words.get(bot()->getStorage(s), wrd , &nick);
		if (value.isEmpty())
			reply(s, "I don't know");
		else
		{
			QString dest=parser.nextToken();
			if (!dest.isEmpty())
			{
				if (getRole(s)<ROLE_MODERATOR)
				{
					reply(s, "You should be moderator to do this");
					return true;
				}
				QString jid=bot()->getJID(s, dest);
				if (jid.isEmpty())
				{
					reply(s, "Nick not found: "+dest);
					return true;
				}
				s->addAttribute("from", jid.toStdString());
				s->finalize();
			}
			QString toSay=QString("%1 says that %2 = %3").arg(nick).arg(wrd).arg(value);
			reply(s, toSay,!dest.isEmpty());
		}
		return true;
	}

	if (cmd=="SHOWPRIV" || cmd=="SHOWPRIVSILENT")
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
		if (cmd!="SHOWPRIVSILENT")
			reply(s, "Ok");
		s->addAttribute("from", jid.toStdString());
		s->finalize();
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
			QString name=arg.section('=', 0, 0).trimmed();
			QString value=arg.section('=', 1).trimmed();
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
			if (getRole(s)<ROLE_OWNER)
			{
				reply(s,"Only owner can do this");
			}
			else
			{
				words.clear(bot()->getStorage(s));
				reply(s, "Ok");
			}
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

