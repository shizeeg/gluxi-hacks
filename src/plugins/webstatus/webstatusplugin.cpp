#include "webstatusplugin.h"
#include "webstatusthread.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/datastorage.h"
#include "base/messageparser.h"

#include <gloox/stanza.h>

#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QtDebug>

WebstatusPlugin::WebstatusPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "INVITE" << "INFO" << "AVAILABLE" << "AWAY" 
		<< "CHAT" << "DND" << "UNAVAILABLE" << "XA";

	url=DataStorage::instance()->getString("webstatus/url");

	thread=new WebStatusThread();
	thread->start();
}


WebstatusPlugin::~WebstatusPlugin()
{
	thread->stop();
	thread->terminate();
	thread->wait(1);
	delete thread;
	thread=0;
}

bool WebstatusPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.joinBody();
	if (cmd=="INVITE")
	{
		if (!isFromBotOwner(s,true))
			return true;
			
		if (arg.isEmpty())
		{
			reply(s,"Usage: INVITE <JID>");
			return true;
		}
		QSqlQuery query;
		query.prepare("INSERT INTO webstatus (jid, hash) VALUES (?, MD5(?))");
		query.addBindValue(arg);
		query.addBindValue(arg);
		if (!query.exec())
		{
			reply(s,"Failed: "+query.lastError().text());
			return true;
		}
		gloox::Stanza* st;
		st=gloox::Stanza::createSubscriptionStanza(arg.toStdString(), 
			"GluxiBot WebStatus service", gloox::StanzaS10nSubscribe);
		bot()->client()->send(st);
		reply(s,"Subscription request sent");
		return true;
	}
	if (cmd=="INFO")
	{
		QString jid=QString::fromStdString(s->from().bare());
		QSqlQuery query;
		query.prepare("SELECT hash, available, away, chat, dnd, unavailable, xa"
			" FROM webstatus WHERE jid=?");
		query.addBindValue(jid);
		if (!query.exec() || !query.next())
		{
			reply(s,"Failed. Probably not registered");
			return true;
		}
		QString myUrl=QString(url).arg(query.value(0).toString());
		QString res=QString("WebStatus URL: %1\n"
			"Available: %2\n"
			"Away: %3\n"
			"Chat: %4\n"
			"Dnd: %5\n"
			"Unavailable: %6\n"
			"Xa: %7\n")
			.arg(myUrl)
			.arg(query.value(1).toString())
			.arg(query.value(2).toString())
			.arg(query.value(3).toString())
			.arg(query.value(4).toString())
			.arg(query.value(5).toString())
			.arg(query.value(6).toString());
		reply(s,res);

		return true;
	}
	if (cmd=="AVAILABLE" || cmd=="AWAY" || cmd=="CHAT" || cmd=="DND"
		|| cmd=="UNAVAILABLE" || cmd=="XA")
	{
		if (arg.isEmpty())
		{
			reply(s,QString("Usage: %1 <URL>").arg(cmd));
			return true;
		}
		QString argU=arg.toUpper();
		if (!(argU.startsWith("HTTP://") || argU.startsWith("HTTPS://")))
		{
			reply(s,"Only http:// and https:// are supported");
			return true;
		}

		QString jid=QString::fromStdString(s->from().bare());
		qDebug() << "WebStatus: Updating conf for " << jid;
		QSqlQuery query;
		query.prepare(QString("UPDATE webstatus SET %1=? WHERE jid=?").arg(cmd.toLower()));
		query.addBindValue(arg);
		query.addBindValue(jid);
		if (!query.exec()|| !query.numRowsAffected())
		{
			reply(s,"Failed. Probably not registered");
			return true;
		}
		reply(s,"Ok");
		return true;
	}
	return false;
}

bool WebstatusPlugin::canHandlePresence(gloox::Stanza* s)
{
	// Accept all presences.. Reduce SQL usage
	return true;
}

void WebstatusPlugin::onPresence(gloox::Stanza *s)
{
	QString from=QString::fromStdString(s->from().bare());
	QSqlQuery query;
	query.prepare("SELECT jid FROM webstatus WHERE jid=?");
	query.addBindValue(from);
	if (!query.exec() || !query.next())
		return;

	qDebug() << "Got presence from " << from;
	QString status=getPresence(s->presence());
	if (status.isEmpty())
		return;
	query.prepare("UPDATE webstatus SET status=? WHERE jid=?");
	query.addBindValue(status);
	query.addBindValue(from);
	query.exec();
}

