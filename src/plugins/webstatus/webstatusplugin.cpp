#include "webstatusplugin.h"
#include "base/gluxibot.h"

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
}


WebstatusPlugin::~WebstatusPlugin()
{}

bool WebstatusPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();
	QString arg=body.section(' ',1);
	
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
	QString status=getPresence(s->show());
	if (status.isEmpty())
		return;
	query.prepare("UPDATE webstatus SET status=? WHERE jid=?");
	query.addBindValue(status);
	query.addBindValue(from);
	query.exec();
}

