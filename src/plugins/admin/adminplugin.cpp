#include "adminplugin.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/asyncrequest.h"
#include "base/rolelist.h"
#include "base/messageparser.h"
#include "base/glooxwrapper.h"
#include "base/datastorage.h"

#include <gloox/client.h>
#include <gloox/stanza.h>

#include <QtDebug>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>

QString restrictedTerms[]={"DELETE", "UPDATE", "TRUNCATE", "DROP", "CREATE", 
			"GRANT", "ALTER", "INSERT", "TRANSACTION", ""};

const int TAB_SIZE=1;

AdminPlugin::AdminPlugin(GluxiBot *parent)
		:
	BasePlugin(parent)
{
	commands << "QUIT" << "ROLES" << "ASYNCCOUNT" << "ASYNCLIST" << "PRESENCE"
			<< "PRESENCEJID" << "SUBSCRIBE" << "SQL";
}

AdminPlugin::~AdminPlugin()
{
}

bool AdminPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s);
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();

	if (cmd=="QUIT")
	{
		if (isFromBotOwner(s))
		{
			reply(s, "Ok");

			bot()->onQuit("QUIT command from bot owner");
		}
		else
		{
			reply(s, "Only owner can do this");
		}
		return true;
	}
	if (cmd=="ROLES")
	{
		if (isFromBotOwner(s))
		{
			QString res;
			RoleList *list=bot()->roles();
			int cnt=list->keys().count();
			for (int i=0; i<cnt; i++)
			{
				res+=QString("\n%1: %2").arg(list->keys()[i]).arg(list->get(list->keys()[i]));
			}
			reply(s, QString("Roles: %1").arg(res));
		}
		else
		{
			reply(s, "Only owner can do this");
		}
		return true;
	}

	if (cmd=="ASYNCCOUNT")
	{
		reply(s, QString("Async requests count: %1").arg(bot()->asyncRequests()->count()));
		return true;
	}

	if (cmd=="ASYNCLIST")
	{
		if (isFromBotOwner(s))
		{
			int cnt=bot()->asyncRequests()->count();
			if (cnt==0)
				reply(s, "No async requests found");
			else
			{
				QString res;
				for (int i=0; i<cnt; i++)
				{
					AsyncRequest *req=bot()->asyncRequests()->at(i);
					QString plugin;
					QString stanza;
					if (req->plugin())
						plugin=req->plugin()->name();
					else
						plugin="UNKNOWN";
					if (req->stanza())
					{
						stanza=QString::fromStdString(req->stanza()->body());
						stanza.replace('\n', ' ');
					}
					res+=QString("\n%1: %2").arg(plugin).arg(stanza);
				}
				reply(s, "Active async requests: "+res);
			}
		}
		else
			reply(s, "Only owner can do this");
		return true;
	}

	if (cmd=="PRESENCE")
	{
		if (!isFromBotOwner(s))
		{
			reply(s, "Only owner can do this");
			return true;
		}
		QString pr=parser.nextToken().toUpper();
		gloox::Presence presence=presenceFromString(pr);
		if (presence==gloox::PresenceUnknown)
		{
			reply(s, "Available presences: available, away, xa, dnd, chat");
			return true;
		}
		QString status=parser.nextToken();
		bot()->client()->setPresence(presence, status, bot()->getPriority());
		return true;
	}

	if (cmd=="PRESENCEJID")
	{
		if (!isFromBotOwner(s))
		{
			reply(s, "Only owner can do this");
			return true;
		}

		QString target=parser.nextToken();

		QString pr=parser.nextToken().toUpper();
		gloox::Presence presence=presenceFromString(pr);
		if (presence==gloox::PresenceUnknown)
		{
			reply(s, "Syntax: presencejid JID PRESENCE STATUS.\n"
				"Available presences: available, away, xa, dnd, chat");
			return true;
		}

		QString status=parser.nextToken();
		bot()->client()->setPresence(target, presence, status);
		return true;
	}

	if (cmd=="SUBSCRIBE")
	{
		if (!isFromBotOwner(s))
		{
			reply(s, "Only owner can do this");
			return true;
		}
		QString target=parser.nextToken();
		if (target.isEmpty())
		{
			reply(s, "Syntax: subscribe JID");
		}
		else
		{	
			gloox::Stanza* st;
			st=gloox::Stanza::createSubscriptionStanza(target.toStdString(), 
					"GluxiBot subscribed", gloox::StanzaS10nSubscribed);
			bot()->client()->send(st);
			st=gloox::Stanza::createSubscriptionStanza(target.toStdString(),
					"GluxiBot subscription request", gloox::StanzaS10nSubscribe);
			bot()->client()->send(st);
			bot()->client()->setPresence(target, gloox::PresenceChat, QString::null);
			reply(s, "Request sent");
		}
		return true;
	}
	
	if (cmd=="SQL")
	{
		if (!isFromBotOwner(s))
		{
			reply(s, "Only bot owner can do this");
			return true;
		}
		
		QString query=parser.joinBody();
		QString queryUp=query.toUpper();
		int idx=0;
		while (!restrictedTerms[idx].isEmpty())
		{
			int ps=queryUp.indexOf(restrictedTerms[idx]+" ");
			if (ps>=0)
			{
				reply(s, QString("Restricted term found: %1 (at %2)")
						.arg(restrictedTerms[idx]).arg(ps));
				return true;
			}
			++idx;
		}
		
		QSqlQuery sql=DataStorage::instance()->prepareQuery(query);
		if (!sql.exec())
		{
			reply(s,sql.lastError().text());
			return true;
		}
		QSqlRecord rec = sql.record();
		
		int cnt=rec.count();
		QVector<QVector<QString> > resultTable;
		
		QVector<QString> line;
		line.reserve(cnt);
		for (int i=0; i<cnt; ++i)
		{
			line.append(rec.fieldName(i));
		}
		
		resultTable.append(line);
		line.clear();
		
		int totalRows=0;
		while (sql.next())
		{
			line.clear();
			for (int i=0; i<cnt; ++i)
				line.append(sql.value(i).toString());
			resultTable.append(line);
			if (++totalRows==10)
				break;
		}
		
		QVector<int> colWidth(cnt);
		colWidth.fill(0);
		
		for (int row=0; row<resultTable.count(); ++row)
			for (int col=0; col<cnt; ++col)
				if (resultTable[row][col].length()>colWidth[col])
					colWidth[col]=resultTable[row][col].length();
				
		QString result;
		for (int row=0; row<resultTable.count(); ++row)
		{
			QString line;
			QVector<QString> rowVector=resultTable[row];
			for (int col=0; col<cnt; ++col)
			{
				QString term=rowVector[col];
				int l=term.length();
				int maxL=colWidth[col];
				line+=term;
				line+=QString().fill(' ',(maxL-l)/TAB_SIZE + 1);
			}
			result+="\n"+line;
		}		
		
		reply(s,QString("Result:")+result);
		return true;
	}

	return false;
}

gloox::Presence AdminPlugin::presenceFromString(const QString& pr)
{
	gloox::Presence presence;
	if (pr=="AVAILABLE")
		presence=gloox::PresenceAvailable;
	else if (pr=="AWAY")
		presence=gloox::PresenceAway;
	else if (pr=="XA")
		presence=gloox::PresenceXa;
	else if (pr=="DND")
		presence=gloox::PresenceDnd;
	else if (pr=="CHAT")
		presence=gloox::PresenceChat;
	else
		presence==gloox::PresenceUnknown;
	return presence;
}
