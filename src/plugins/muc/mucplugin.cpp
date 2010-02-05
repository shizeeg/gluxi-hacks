#include "mucplugin.h"
#include "conference.h"
#include "nicklist.h"
#include "alist.h"
#include "alistitem.h"
#include "jid.h"
#include "jidstat.h"
#include "config/mucconfigurator.h"
#include "nickasyncrequest.h"
#include "muchistory.h"
#include "cleanasyncrequest.h"

#include "base/common.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/datastorage.h"
#include "base/rolelist.h"
#include "base/messageparser.h"
#include "base/asyncrequest.h"
#include "base/asyncrequestlist.h"
#include "base/vcardwrapper.h"

#include <QtDebug>
#include <QRegExp>
#include <QTimer>

#include <iostream>

#include <gloox/stanza.h>
#include <assert.h>

MucPlugin::MucPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "WHEREAMI" << "NICK" << "IDLE" << "KNOWN" << "JOIN" << "LEAVE"
			<< "KICK" << "VISITOR" << "PARTICIPANT" << "MODERATOR" << "BAN"
			<< "BANJID" << "UNBAN" << "NONE" << "MEMBER" << "ADMIN" << "OWNER";
	commands << "ABAN" << "AKICK" << "AVISITOR" << "ACMD" << "AMODERATOR" << "APARTICIPANT" << "AFIND"
			<< "ATRACE"  << "SEEN" << "SEENEX" << "CLIENTS" << "SETNICK" << "CHECKVCARD" << "ROLE" << "VERSION";
	commands << "HERE" << "STATUS" << "AGE" << "AGESTAT";

	commands << "REPORT" << "MISSING" << "STAT";

	commands << "POKE" << "REALJID" << "INVITE" << "CLEAN";
	pluginId=1;

	// Plugin should be able to modify self-messages so they will be
	// unparseable by other plugins
	priority_=20;
	lazyOffline=DataStorage::instance()->getInt("muc/lazyoffline");

	int leaveCheckInterval=DataStorage::instance()->getInt("muc/leave_checkinterval");
	if (leaveCheckInterval>0)
	{
		leaveCheckInterval*=1000;
		QTimer* timer=new QTimer(this);
		connect(timer,SIGNAL(timeout()), SLOT(sltAutoLeaveTimerTimeout()));
		timer->start(leaveCheckInterval);
	}

}

MucPlugin::~MucPlugin()
{
}

void MucPlugin::onConnect()
{
	QStringList list=Conference::autoJoinList();
	QStringListIterator it(list);
	while (it.hasNext())
		join(it.next());
}

void MucPlugin::onDisconnect()
{
	qDebug() << "MucPlugin: cleaning for onDisconnect()";
	confInProgress.clear();
	if (lazyOffline)
		conferences.lazyClear();
	else
		conferences.clear();
}

bool MucPlugin::canHandlePresence(gloox::Stanza* s)
{
	QString from=QString::fromStdString(s->from().bare());
	QString fromFull=QString::fromStdString(s->from().full());
	int ret= (conferences.byName(from)
			|| (confInProgress.indexOf(getConfExp(from))>=0));
	return ret;
}

bool MucPlugin::isMyMessage(gloox::Stanza*s)
{
	QString jid=QString::fromStdString(s->from().bare());
	Conference* conf=conferences.byName(jid);
	if (!conf)
		return false;
	if (s->from()==s->to())
		return false;
	QString res=QString::fromStdString(s->from().resource());
	if (res.isEmpty() || conf->nick()==res)
		return true;
	return false;
}

QString MucPlugin::getMyNick(gloox::Stanza* s)
{
	Conference *conf=getConf(s);
	if (!conf)
		return BasePlugin::getMyNick(s);
	return conf->nick();
}

QString MucPlugin::resolveMyNick(gloox::Stanza* s)
{
	Conference *conf=getConf(s);
	if (!conf)
		return QString::null;
		return conf->nick();
	}

bool MucPlugin::canHandleMessage(gloox::Stanza* s, const QStringList& flags)
{
	Q_UNUSED(flags)

	// 	std::cout << s->xml() << std::endl;
	if (isOfflineMessage(s))
	{
		if (lazyOffline)
		{
			Conference *conf=getConf(s);
			if (conf!=0)
			{
				if (!conf->validated())
				{
					conf->cleanNonValidNicks();
					conf->setValidated(true);
				}
			}
		}
		return false;
	}
	if (allMessages())
		return true;

	MessageParser parser(s, getMyNick(s));
	parser.nextToken();

	Conference *conf=getConf(s);
	if (!conf)
	{
		// Join/leave commands from owner
		QString cmd=parser.nextToken().toUpper();
		if (cmd=="JOIN" || cmd=="LEAVE" || cmd=="WHEREAMI")
			return true;

		// Possible captcha request from conference server
		QString from=QString::fromStdString(s->from().full());
		if (from.indexOf('@')<0 && from.indexOf('/') < 0)
		{
			return true;
		}

		return false;
	}
	Nick *n=getNick(s);
	if (!n)
	{
		qDebug() << "MUC: getNick() returns 0L";
		return false;
	}
	if (n->nick()==conf->nick() && !(s->from()==s->to()))
	{
		logMessageStanza(s, conf, flags);
		qDebug() << "MUC: Self message ignored";
		// Avoid processing of such message by other plugins.
		// Currently just add invalid "to" and clear body :)
		s->addAttribute("from","");
		s->addAttribute("to","");
		gloox::Tag* tag=s->findChild("body");
		if (tag)
			tag->setCData("");
		s->finalize();
		return false;
	}
	return true;
}

void MucPlugin::onPresence(gloox::Stanza* s)
{
	QString nick=QString::fromStdString(s->from().resource());

	QString role=getItem(s, "role");
	QString type=QString::fromStdString(s->findAttribute("type"));

	QString confFull=QString::fromStdString(s->from().full());
	if (type=="error")
	{
		qDebug()
				<< "[MUC] Got type='error' in onPresence. Looks like we can't join conference: " << confFull << "Nick: " << nick;
		int idx=confInProgress.indexOf(getConfExp(confFull));
		if (idx>=0)
			confInProgress.removeAt(idx);
		QString registeredNick=DataStorage::instance()->getString("muc/nick");
		if (nick!=registeredNick)
		{
			// Try to rejoin with default nick
			confFull=confFull.section('/',0,0);
			join(confFull);
		}
		else
		{
			gloox::JID ownerJid(DataStorage::instance()->getStdString("access/owner"));
			gloox::Stanza *outgoing=gloox::Stanza::createMessageStanza(ownerJid,
				QString("Error presence from \"%1\". Looks like I can't join:\n%2")
				.arg(QString::fromStdString(s->from().bare()))
				.arg(QString::fromStdString(s->xml())).toStdString());
			bot()->client()->send(outgoing);
			Conference::disableAutoJoin(confFull.section('/',0,0));
		}
		return;
	}

	Conference* conf=getConf(s);

	//FIXME: Detect, what we shoud do if conf don't exists
	if (!conf)
	{
		// TODO: Use XEP-0045 for this about <status/>. Currently ejabberd
		// don't send <status code='110'/> for our nick
		int ps=confInProgress.indexOf(getConfExp(confFull));
		if (ps<0)
		{
			qDebug()
					<< "[!!!!!] Probably bug. Can't find conf in confInProgress.";
			return;
		}
		confFull=confInProgress[ps];;

		conf=new Conference(confFull.section('/',0,0),confFull.section('/',1,1), lazyOffline);
		MucConfigurator* configurator=new MucConfigurator(confFull,StorageKey(pluginId, conf->id()));
		conf->setConfigurator(configurator);
		conferences.append(conf);
		confInProgress.removeAt(ps);

		if (lazyOffline)
		{
			conf->loadOnlineNicks();
		}
	}

	Nick *n=getNick(s);
	if (lazyOffline && n && n->validateRequired())
	{
		QString jid=getItem(s, "jid").section('/', 0, 0);
		if (n->jidStr()!=jid)
		{
			qDebug() << QString("[%1] Nick \"%2\" changed JID: %3 -> %4").arg(conf->name()).arg(n->nick()).arg(n->jidStr()).arg(jid);
			n->setValidateRequired(false);
			n->updateLastActivity();
			n->commit();
			bot()->roles()->remove(QString::fromStdString(s->from().full()));
			conf->nicks()->remove(n);
			n=0;
		}
	}

	bool newNick=false;
	if (!n)
	{
		qDebug() << "MUC::handlePresence: new Nick";
		if (role=="none" || type=="unavailable")
			return;

		newNick=true;
		n=new Nick(conf, nick,getItem(s,"jid"));
		conf->nicks()->append(n);

		JidStat *stat = n->jidStat();
		if (stat)
		{
			// Just presence?
			QString show = getPresence(s->presence());
			QString status = QString::fromStdString(s->status());
			if (!status.isEmpty())
				show += QString(" (%1)").arg(status);
			stat->setLastAction(ActionJoin, show);

			if (conf->history())
				conf->history()->log(n, NULL, ActionJoin, show, false,
						QString("%1|%2").arg(role, getItem(s, "affiliation")));
		}

		if (nick==conf->nick())
		{
			// Got own presence after renaming

		}
	}
	else
	{
		if (n->validateRequired())
			newNick=true;
	}

	if (role=="none" || type=="unavailable")
	{
		n->updateLastActivity();
		n->commit();
		if (n->nick()==conf->nick())
		{
			//Check for possible renaming
			int status=getStatus(s);
			if (status==303)
			{
				QString newNick=getItem(s, "nick");
				qDebug() << "Renaming done: "+newNick;
				conf->nicks()->remove(n);
				conf->setNick(newNick);
			}
			else
			{
				qDebug() <<"!!! I'm kicked/banned";
				conf->setAutoJoin(false);
				conferences.remove(conf);
			}
		}
		else
		{
			qDebug() << "!!!!!! Removing nick";
			JidStat *stat = n->jidStat();
			if (stat)
			{
				int statusCode = getStatus(s);
				if (statusCode == 307 || statusCode == 301)
				{
					ActionType actType = statusCode == 307
						? ActionKick : ActionBan;
					QString reason = getReason(s);
					stat->setLastAction(actType, reason);

					if (conf->history())
						conf->history()->log(n, NULL, actType, reason, false);
				}
				else
				{
					// Just presence?
					QString show = QString::fromStdString(s->status());
					stat->setLastAction(ActionLeave, show);
					if (conf->history())
						conf->history()->log(n, NULL, ActionLeave, show, false);
				}
			}
			conf->nicks()->remove(n);
			conf->alistTraceList()->removeAll(QString::fromStdString(s->from().full()));
		}
		bot()->roles()->remove(QString::fromStdString(s->from().full()));
	}
	else
	{
		JidStat *stat = n->jidStat();
		if (stat && !newNick)
		{
			bool recorded = false;
			if (n->role() != role)
			{
				ActionType actType = ActionNone;
				if (role == "moderator")
					actType = ActionModerator;
				else if (role == "participant")
					actType = ActionParticipant;
				else if (role == "visitor")
					actType = ActionVisitor;
				QString reason = getReason(s);
				stat->setLastAction(actType, reason);
				if (conf->history())
					conf->history()->log(n, NULL, actType, reason, false);
				recorded = true;
			}
			if (n->affiliation() != getItem(s, "affiliation"))
			{
				QString aff = getItem(s, "affiliation");
				ActionType actType = ActionNone;
				if (aff == "none")
					actType = ActionNoAffiliation;
				else if (aff == "member")
					actType = ActionMember;
				else if (aff == "administrator")
					actType = ActionAdministrator;
				else if (role == "owner")
					actType = ActionOwner;
				QString reason = getReason(s);
				stat->setLastAction(actType, reason);
				if (conf->history())
					conf->history()->log(n, NULL, actType, reason, false);
				recorded = true;
			}

			if (!recorded)
			{
				// Just presence?
				QString show = getPresence(s->presence());
				QString status = QString::fromStdString(s->status());
				if (!status.isEmpty())
					show += QString(" (%1)").arg(status);
				stat->setLastAction(ActionPresence, show);
				if (conf->history())
					conf->history()->log(n, NULL, ActionPresence, show, false);
			}
		}

		n->setAffiliation(getItem(s, "affiliation"));
		n->setJid(getItem(s, "jid"));
		n->setRole(role);
		n->setShow(getPresence(s->presence()));
		n->setStatus(QString::fromStdString(s->status()));
		n->updateLastActivity();
		n->commit();

		QString confJid=QString::fromStdString(s->from().full());
		bot()->roles()->insert(confJid, n->jidStr().section('/', 0, 0));
		bot()->roles()->update(confJid, RoleList::calc(n->role(), n->affiliation()));
		/*		if (!confJid.isEmpty())
		 {
		 if (bot()->owners()->indexOf(n->jid().section('/',0,0))>=0)
		 bot()->tmpOwners()->append(confJid);

		 }
		 */
		conf->removeExpired();
		if (newNick || conf->configurator()->isCheckAlistsEveryPresence())
			checkMember(s, conf, n);

		if (newNick)
		{
			if (conf->configurator()->isDevoiceNoVCard())
				requestVCard(s, conf, n);
			if (conf->configurator()->isQueryVersionOnJoin())
			{
				requestVersion(s, conf, n);
			}
		}
	}
}

void MucPlugin::logMessageStanza(gloox::Stanza *s, Conference *conf, const QStringList& flags)
{
	if (!conf)
		conf = getConf(s);

	if (conf && conf->history())
	{
		Nick *n = getNick(s);
		if (n)
		{
			QString msgBody = QString::fromStdString(s->body());
			bool isPrivate = s->findAttribute("type") != "groupchat";
			if (!msgBody.isEmpty())
			{
				ActionType actType = ActionMessage;

				if (flags.contains("glooxbot_alias")
						|| s->findAttribute("glooxbot_alias") == "true")
					actType = ActionExpandedAlias;

				Nick *dstNick = NULL;
				if (msgBody.contains(':'))
				{
					// Find destination nick
					QString dstNickStr = msgBody.section(':', 0, 0);
					dstNick=conf->nicks()->byName(dstNickStr);
				}

				conf->history()->log(n, dstNick, actType, msgBody, isPrivate);
				n->updateLastActivity(true);
			}

			QString subject = QString::fromStdString(s->subject());
			if (!subject.isEmpty())
				conf->history()->log(n, NULL, ActionSubject, subject, isPrivate);
		}
	}
}

bool MucPlugin::parseMessage(gloox::Stanza* s, const QStringList& flags)
{
	if (isOfflineMessage(s))
		return true;

	MessageParser parser(s, getMyNick(s));
	QString msgPrefix=parser.nextToken().toUpper();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.nextToken();

	QString nickName=QString::fromStdString(s->from().resource());

	Conference* conf=getConf(s);

	if (!flags.contains("acmd"))
		logMessageStanza(s, conf, flags);

	if (parser.isForMe())
	{

		if (conf==0 && nickName.isEmpty())
		{
			QString from=QString::fromStdString(s->from().full()).toLower();
			if (from.indexOf('@')<0 && from.indexOf('/') < 0)
			{
				from=QString("@%1/").arg(from);
				for (QStringList::iterator it=confInProgress.begin(); it
						!=confInProgress.end(); ++it)
				{
					QString confName=*it;
					if (confName.toLower().indexOf(from)>0)
					{
						//Captcha request
						gloox::JID ownerJid(DataStorage::instance()->getStdString("access/owner"));
						gloox::Stanza *outgoing=
								gloox::Stanza::createMessageStanza(ownerJid,
										s->body());
						bot()->client()->send(outgoing);
						return true;
					}
				}
			}
		}

		if (cmd=="JOIN")
		{
			if (!isFromBotOwner(s))
				return true;
			if (arg.indexOf('@')<0 || arg.indexOf('/')>=0)
			{
				reply(s, "Conference should be like \"room@server\"");
				return true;
			}
			reply(s, "Ok");
			//!!!!!!!!!!!!!!!!!!!!
			join(arg);
			return true;
		}
		if (cmd=="LEAVE")
		{
			if (!isFromBotOwner(s, false))
			{
				if (isFromConfOwner(s))
				{
					if (!arg.isEmpty())
					{
						reply(s,
								"You can only use \"leave\" for default (your) room");
						return true;
					}
				}
				else
					return true;
			}
			if (arg.isEmpty() && conf)
				arg=conf->name();

			if (arg.indexOf('@')<0 || arg.indexOf('/')>=0)
			{
				reply(s, "Conference should be like \"room@server\"");
				return true;
			}
			reply(s, "Ok");
			leave(arg);
			return true;
		}

		if (cmd=="WHEREAMI")
		{
			QStringList confList;
			for (int i=0; i<conferences.count(); i++)
				confList << (conferences[i]->name().section('@', 0, 0)+"@");
			reply(
					s,
					QString("Currently I'm spending time at %1").arg(confList.join(", ")));
			if (!confInProgress.isEmpty())
				reply(s, QString("Conferences in-progress:\n"
						+confInProgress.join("\n")));
			return true;
		}
	}

	if (!conf)
	{
		return true;
	}

	Nick *nick=getNick(s);
	if (!nick)
	{
		reply(s, QString("Looks like %1 gone...").arg(nickName));
		return true;
	}
	nick->updateLastActivity();
	nick->commit();

	QString msgBody = QString::fromStdString(s->body());

	if (!flags.contains("glooxbot_alias"))
	{
		JidStat *stat = nick->jidStat();
		if (stat)
		{
			if (!msgBody.isEmpty())
				stat->statMessage(msgBody);
			QString subject = QString::fromStdString(s->subject());
			if (!subject.isEmpty())
				stat->statSubject(subject);
		}
	}

	if (msgBody.contains(':'))
	{
		// Calc also highlights
		QString dstNickStr = msgBody.section(':', 0, 0);
		Nick *dstNick=conf->nicks()->byName(dstNickStr);
		if (dstNick && nick != dstNick && dstNick->jidStat())
			dstNick->jidStat()->statReply();
	}

	checkMember(s, conf, nick);

	if (msgPrefix!=prefix() || !parser.isForMe())
	{
		myShouldIgnoreError=1;
		return false;
	}
	
	if (cmd=="CLEAN")
	{
  		CleanAsyncRequest *req = new CleanAsyncRequest(this, new gloox::Stanza(s), arg);
		bot()->asyncRequests()->append(req);
		req->exec();
		return true;
	}
	
	if (cmd=="INVITE")
	{
		if( arg.isEmpty() )
		{
			reply(s, "!muc invite <nick> or <jid> [reason]");
			return true;
		}

		QString n	= arg;
		QString reason	= parser.nextToken();
		QString msg	= invite(s, n, reason);
		reply(s, msg);
		return true;
	}

	if (cmd=="REALJID")
	{
		if (!isFromConfModerator(s)) {
			reply(s,"You should be moderator to do this");
			return true;
		}

		QStringList jids;
		Nick* nick=getNick(s, arg);
		if (!nick)
		{
			jids = Nick::nickToJids(getConf(s), arg);
			if(!jids.isEmpty())
			{
				reply(s, QString("\nNick:\t%1\nJID(s):\t%2")
					.arg(arg).arg(jids.join("\n")), true);
				reply(s, "OK.");
				return true;
			}
		}
		else
		{
			reply(s, QString("\nNick:\t%1\nJID:\t%2")
				.arg(arg)
				.arg(nick->jidStr().section("/",0,0)), true);
			reply(s, "OK.");
			return true;
		}
		reply(s, "I can't remember his/her jid.");
		return true;
	}

	if (cmd=="POKE")
	{
		QStringList nickList;
		int cnt=conf->nicks()->count();
		QStringList replys; replys
		<< "облил %1 ледяной водой"
		<< "закидал %1 тухлыми помидорами"
		<< "шарахнул  %1 веслом по голове"
		<< "ткнул %1 в глаз"
		<< "постукал %1 головой апстенку"
		<< "дал %1 йаду"
		<< "slaps %1 around a bit with a large trout"
		<< "приковал наручниками к кровати %1 и заставил слушать Децла. МНОГО ДЕЦЛА!"
		<< "шарахнул %1 веслом по голове"
		<< "потыкал %1 палочкой"
		<< "целится плюсомётом в %1"
		<< "тыкает %1 со словами \"нуу, пратииивный\""
		<< "неожиданно проорал \"БУУУУ!\" в ухо %1"
		<< "случайно уронил кирпич на голову %1"
		<< "попрыгал с бубном вокруг %1"
		<< "размахивает руками перед лицом %1"
		<< "воззвал к %1"
		<< "трясёт %1 за плечи"
		<< "кинул нож в сторону %1";

		srand ( time(NULL) );
		int r = rand() % replys.count();
		QString msg = replys[r];

		if( arg.isEmpty() )
		{
			reply(s, "!muc poke <nick>");
			return true;
		}
		if ( arg.length() > 35 )
		{
			reply(s, "шибко умный, да? ]:->");
			return true;
		}
		if ( s->subtype() == gloox::StanzaMessageChat ) {
			reply(s, ":-P");
			return true;
		}
		QString nickName = QString::fromStdString(s->from().resource());

		if ( arg.compare( getMyNick(s), Qt::CaseInsensitive ) == 0 )
		{
			reply(s, "Ы?");
			return true;
		}
		if ( arg.compare(nickName, Qt::CaseInsensitive) == 0 )
		{
			reply(s, "мазохист? :D");
			return true;
		}

		for (int i=0; i<cnt; i++)
		{
			if( arg.compare( conf->nicks()->at(i)->nick(), Qt::CaseInsensitive ) == 0 )
			{
				reply(s, QString("/me " + msg)
					.arg(conf->nicks()->at(i)->nick()), false, false);
				return true;
			}
		}

		reply(s, "а он тут? :-O");
		return true;
	}

	if (cmd=="HERE")
	{
		QStringList nickList;
		int cnt=conf->nicks()->count();
		for (int i=0; i<cnt; i++)
			nickList << conf->nicks()->at(i)->nick();
		reply(s, QString("I can see %1 users here: %2").arg(cnt)
		.arg(nickList.join(", ")));
		return true;
	}

	if (cmd=="NICK")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		Jid* jid=n->jid();

		QString jidCreated;
		if (jid)
		{
			jidCreated=jid->created().toString(Qt::LocaleDate);
		}
		else
		{
			jidCreated="unknown";
		}

		QString
				nickInfo=
						QString("Nick \"%1\": Affiliation: %2; Role: %3; Registered: %4; Joined: %5; Idle: %6; Role: %7; Age: %8; Status: %9 (%10)")
						.arg(n->nick())
						.arg(n->affiliation())
						.arg(n->role())
						.arg(jidCreated)
						.arg(n->joined().toString(Qt::LocaleDate))
						.arg(secsToString(n->lastActivity().secsTo(QDateTime::currentDateTime())))
						.arg(getRoleForNick(conf, n))
						.arg(n->jid() ? n->jid()->created().secsTo(QDateTime::currentDateTime()) : 0)
						.arg(n->show())
						.arg(n->status());
		reply(s, nickInfo);
		return true;
	}

	if (cmd=="IDLE")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		reply(s, QString("Idle for \"%1\" is %2").arg(n->nick()).arg(secsToString(n->lastActivity().secsTo(QDateTime::currentDateTime()))));
		return true;
	}

	if (cmd=="ROLE")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		reply(s,QString("Role for \"%1\" is %2").arg(n->nick()).arg(getRoleForNick(conf, n)));
		return true;
	}

	if (cmd=="VERSION")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		QString res=n->versionName();
		if (!n->versionClient().isEmpty())
			res+=QString(" %1").arg(n->versionClient());
		if (!n->versionOs().isEmpty())
			res+=QString(" // %1").arg(n->versionOs());
		if (res.isEmpty())
			reply(s, QString("No version stored for \"%1\"").arg(n->nick()));
		else
			reply(s, QString("%1 uses %2").arg(n->nick()).arg(res));
		return true;
	}

	if (cmd=="CHECKVCARD")
	{
		Nick *n=getNick(s);
		if (!n)
			return true;
		requestVCard(s,conf,n);
		reply(s,"Ok");
		return true;
	}

	if (cmd=="SEEN" || cmd=="SEENEX")
	{
	  if( arg.isEmpty() ) {
	    Nick *n = getNick(s);
	    if(!n) return true;
	    reply(s, conf->seen(n->nick(), "SEENEX" == cmd));
	    return true;
	  }
		reply(s, conf->seen(arg, "SEENEX" == cmd));
		return true;
	}

	if (cmd == "REPORT")
	{
		QString res = JidStat::queryReport(conf->id(), arg, 10);
		if (!res.isEmpty())
		{
			reply(s, res);
		}
		else
		{
			reply(s, JidStat::availableReports());
		}
		return true;
	}

	if (cmd == "STAT")
	{
		if (arg.toLower() == "list" || arg.isEmpty())
		{
			reply(s, QString("Usage: !muc stat REPORT NICK\nAvailable reports: %1")
					.arg(JidStat::availableTimeReports()));
			return true;
		}
		QString nickStr = parser.nextToken();
		Nick *n = getNickVerbose(s, nickStr);
		if (!n)
			return true;
		nickStr = n->nick();
		JidStat *stat = n->jidStat();
		if (!stat)
		{
			reply(s, QString("Statistic is not collected for \"%1\"").arg(nickStr));
			return true;
		}
		QString report = stat->queryTimeReport(arg);
		if (report.isEmpty())
		{
			reply(s, QString("Unable to prepare report %1.\nAvailable reports: %2")
							.arg(arg, JidStat::availableTimeReports()));
			return true;
		}
		reply(s, QString("Message statistic for \"%1\":%2").arg(nickStr, report));
		return true;
	}

	if (cmd == "MISSING")
	{
		Nick *n=getNick(s);
		MucHistory *h = conf->history();
		if (!h)
		{
			reply(s, "No history available for this room");
			return true;
		}
		QDateTime dateTime = n->lastMessage();
		QList<MucHistory::HistoryItem> resList = h->missingHighlights(n, dateTime);
		if (resList.isEmpty())
		{
			reply(s, QString("No missing highlights for you since %1").arg(dateTime.toString()));
			return true;
		}
		QStringList resStr;
		foreach(MucHistory::HistoryItem item, resList)
		{
			QString msg = QString("[%1] <%2> %3").arg(
					item.dateTime.time().toString(), item.nick, item.message);
			resStr.append(msg);
		}
		reply(s, QString("Missing highlights since %1:\n%2").arg(dateTime.toString(), resStr.join("\n")));
		return true;
	}

	if (cmd=="KNOWN")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		QStringList knownList=n->similarNicks();
		reply(s, QString("\"%1\" is known here as: %2").arg(n->nick()).arg(knownList.join(", ")));
		return true;
	}

	if (cmd=="KICK" || cmd=="VISITOR" || cmd=="PARTICIPANT" || cmd=="MODERATOR")
	{
		if (!isFromConfModerator(s))
			return true;

		if (cmd=="MODERATOR")
		{
			if (getRole(s)<ROLE_ADMIN)
				return true;
		}

		QString target=arg;
		QString reason=parser.nextToken();

		Nick *n=getNickVerbose(s, target);
		if (!n)
			return true;
		if (cmd=="KICK")
			setRole(s, n, "none", reason);
		else if (cmd=="VISITOR")
			setRole(s, n, "visitor", reason);
		if (cmd=="PARTICIPANT")
			setRole(s, n, "participant", reason);
		if (cmd=="MODERATOR")
			setRole(s, n, "moderator", reason);
		return true;
	}

	if (cmd=="BAN" || cmd=="NONE" || cmd=="MEMBER" || cmd=="ADMIN" || cmd
			=="OWNER")
	{
		if (warnImOwner(s))
			return true;

		if (getRole(s) < ROLE_ADMIN)
		{
			reply(s, "You have no rights to edit affiliation. Sorry");
			return true;
		}
		Nick *nick=conf->nicks()->byName(arg);
		if (!nick)
		{
			reply(s, "No nick found: "+arg);
			return true;
		}
		QString reason=parser.nextToken();
		QString affiliation=affiliationByCommand(cmd);

		setAffiliation(conf, nick->jidStr(), affiliation, reason);
		return true;
	}

	if (cmd=="BANJID" || cmd=="UNBAN")
	{
		if (warnImOwner(s))
			return true;

		if (getRole(s) < ROLE_ADMIN)
		{
			reply(s, "You have no rights to edit affiliation. Sorry");
			return true;
		}
		QString jid=arg;
		if (jid.isEmpty())
		{
			reply(s, "No JID specified");
			return true;
		}

		if (!isBareJidValid(jid) && !isServerValid(jid))
		{
			reply(s, "JID is not valid");
			return true;
		}

		QString reason=parser.nextToken();
		QString affiliation=affiliationByCommand(cmd);
		setAffiliation(conf, jid, affiliation, reason);
		reply(s, "done");
		return true;
	}

	if (cmd=="ABAN" || cmd=="AKICK" || cmd=="AVISITOR" || cmd=="AMODERATOR"
		|| cmd=="APARTICIPANT" || cmd=="ACMD" || cmd=="AEDIT" || cmd=="AFIND" || cmd=="AAND")
	{
		if (!isFromConfAdmin(s))
			return true;
		if (flags.contains("acmd"))
		{
			reply(s, "Warn: Modifying alists from acmd is not supported. Ignored", false, false);
			return true;
		}
		parser.back(2);
		return autoLists(s, parser);
	}

	if (cmd=="ATRACE")
	{
		if (!isFromConfAdmin(s))
			return true;
		QString dst=QString::fromStdString(s->from().full());
		if (!conf->alistTraceList()->contains(dst))
		{
			conf->alistTraceList()->append(dst);
			reply(s, "Subscribed to alists trace");
		}
		else
		{
			conf->alistTraceList()->removeAll(dst);
			reply(s, "Unsubscribed from alists trace");
		}
		return true;
	}

	if (cmd=="CLIENTS")
	{
		QString res=conf->clientStat();
		if (res.isEmpty())
			reply(s, "Unable to get client stats");
		else
			reply(s, res);
		return true;
	}
	if (cmd=="SETNICK")
	{
		if (isFromConfOwner(s))
		{
			//Own nick will be changed after ACK presence
			//conf->setNick(arg);
			gloox::Stanza *st=
					gloox::Stanza::createPresenceStanza(gloox::JID(QString(conf->name()+"/"+arg).toStdString()));

			gloox::Tag *tg=new gloox::Tag("x");
			tg->addAttribute("xmlns", "http://jabber.org/protocol/muc");
			st->addChild(tg);
			bot()->client()->send(st);

		}
		else
			reply(s, "never");
		return true;
	}

	if (cmd=="STATUS")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		QString res=n->show();
		if (!n->status().isEmpty())
			res+=QString(" (%1)").arg(n->status());
		reply(s, res);
		return true;
	}

	if (cmd=="AGE")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n)
			return true;
		reply(s,QString::number(n->jid() ? n->jid()->created().secsTo(QDateTime::currentDateTime()) : 0));
		return true;
	}

	if (cmd=="AGESTAT")
	{
		QList<Nick*> tmpList;
		NickList* nickList=conf->nicks();
		for (NickList::iterator it=nickList->begin(); it!=nickList->end(); ++it) {
			Nick* n=*it;
			tmpList.append(n);
		}
		qSort(tmpList.begin(), tmpList.end(), ageLessThan);
		QString r("Age statistic:");
		int idx=1;
		for (QList<Nick*>::iterator it=tmpList.begin(); it!=tmpList.end(); ++it)
		{
			Nick* n=*it;
			if (!r.isEmpty())
				r+="\n";
			r+=QString("%1) %2: %3 (%4)").arg(idx).arg(n->nick())
				.arg(n->jid() ? n->jid()->created().secsTo(QDateTime::currentDateTime()) : 0)
				.arg(n->jid() ? n->jid()->created().toString() : QDateTime::currentDateTime().toString());

			if (++idx>10)
				break;
		}
		reply(s, r);
		tmpList.clear();
		return true;
	}

	return false;
}

Conference* MucPlugin::getConf(gloox::Stanza* s)
{
	QString confName=QString::fromStdString(s->from().bare());
	if (confName.isEmpty())
		confName=QString::fromStdString(s->to().bare());
	Conference* conf=conferences.byName(confName);
	if (!conf)
	{
		return 0;
	}
	return conf;
}

Nick* MucPlugin::getNick(gloox::Stanza* s, const QString& nn)
{
	Conference *conf=getConf(s);
	if (!conf)
	{
		qDebug() << "ERR: getNick() without conf";
		return 0;
	}

	QString nickName;
	if (nn.isEmpty())
		nickName=QString::fromStdString(s->from().resource());
	else
		nickName=nn;

	Nick *nick=conf->nicks()->byName(nickName);
	return nick;
}

bool MucPlugin::isFromConfModerator(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().full());
	return (bot()->roles()->get(jid) >= ROLE_MODERATOR);
}

bool MucPlugin::isFromConfAdmin(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().full());
	return (bot()->roles()->get(jid) >= ROLE_ADMIN);
}

bool MucPlugin::isFromConfOwner(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().full());

	if (bot()->roles()->get(jid) < ROLE_OWNER)
	{
		reply(s, "Only conference owner can do this.");
		return false;
	}
	return true;
}

void MucPlugin::setRole(gloox::Stanza* s, Nick* n, const QString& role,
		const QString& reason)
{
	setRole(getConf(s), n, role, reason);
}

void MucPlugin::setRole(Conference* conf, Nick* n, const QString& role,
		const QString& reason)
{
	if (!conf || !n)
		return;

	gloox::Tag *tag=new gloox::Tag("item");
	tag->addAttribute("nick", QString(n->nick()).toStdString());
	tag->addAttribute("role", role.toStdString());

	gloox::Tag *reas=new gloox::Tag("reason",reason.toStdString());
	tag->addChild(reas);

	gloox::Stanza *st=gloox::Stanza::createIqStanza(conf->name().toStdString(), "someID", gloox::StanzaIqSet,
			"http://jabber.org/protocol/muc#admin", tag);
	bot()->client()->send(st);
}

void MucPlugin::setAffiliation(gloox::Stanza* s, Nick* n,
		const QString& affiliation, const QString& reason)
{
	setAffiliation(getConf(s), n->jidStr(), affiliation, reason);
}

void MucPlugin::setAffiliation(Conference* conf, const QString& jid,
		const QString& affiliation, const QString& reason)
{
	if (!conf || jid.isEmpty())
		return;

	qDebug() << jid;
	gloox::JID glJid(jid.toStdString());
	QString jidStr=QString::fromStdString(glJid.bare());
	qDebug() << jidStr;

	gloox::Tag *tag=new gloox::Tag("item");
	tag->addAttribute("jid", jidStr.toStdString());
	tag->addAttribute("affiliation", affiliation.toStdString());

	gloox::Tag *reas=new gloox::Tag("reason",reason.toStdString());
	tag->addChild(reas);

	gloox::Stanza *st=gloox::Stanza::createIqStanza(conf->name().toStdString(), "someID", gloox::StanzaIqSet,
			"http://jabber.org/protocol/muc#admin", tag);
	bot()->client()->send(st);
}

Nick* MucPlugin::getNickVerbose(gloox::Stanza* s, const QString& nn)
{
	Nick *nick=getNick(s, nn);
	if (!nick)
	{
		reply(s, QString("Nick \"%1\" not found").arg(nn));
	}
	return nick;
}

void MucPlugin::join(const QString& name, const QString& joinerBareJid)
{
	qDebug() << "MucPlugin::join: " << name;
	QString cname;
	QString cnick;
	QString confName=name;
	if (confName.indexOf('/')<0)
	{
		cname=name;
		cnick=DataStorage::instance()->getString("muc/nick");
		confName+="/"+cnick;
	}
	else
	{
		cname=confName.section('/', 0, 0);
		cnick=confName.section('/', 1, 1);
	}

	if (confInProgress.indexOf(getConfExp(confName))>=0)
	{
		qWarning() << "Joining already in progress";
		return;
	}

	confInProgress.append(confName);
	if (!joinerBareJid.isEmpty())
	{

	}

	// Don't create "Conference" object, because it's possible that we can't join it
	// 	Conference *conf=new Conference(cname,cnick);
	// 	conferences.append(conf);
	gloox::Stanza
			*st=
					gloox::Stanza::createPresenceStanza(gloox::JID(confName.toStdString()));

	// Be MUC compatible according to XEP-0045
	gloox::Tag *tg=new gloox::Tag("x");
	tg->addAttribute("xmlns", "http://jabber.org/protocol/muc");
	st->addChild(tg);

	std::cout << st->xml() << std::endl << std::endl;
	bot()->client()->send(st);
}

void MucPlugin::leave(const QString& name)
{
	QString cname;
	QString cnick;
	QString confName=name;
	if (confName.indexOf('/')<0)
	{
		cname=name;
		cnick=DataStorage::instance()->getString("muc/nick");
		confName+="/"+cnick;
	}
	else
	{
		cname=confName.section('/', 0, 0);
		cnick=confName.section('/', 1, 1);
	}

	Conference*conf=conferences.byName(cname);
	if (conf)
	{
		conf->setAutoJoin(FALSE);
		conf->markOffline();
	}


	/*	Conference *conf=new Conference(cname,cnick);
	 conferences.append(conf); */

	gloox::Stanza
			*st=
					gloox::Stanza::createPresenceStanza(gloox::JID(confName.toStdString()));
	st->addAttribute("type", "unavailable");
	bot()->client()->send(st);
}

QString MucPlugin::getItem(gloox::Stanza* s, const QString& name)
{
	std::string nname=name.toStdString();
	std::string res;
	if (s->hasChild("x", "xmlns", "http://jabber.org/protocol/muc#user"))
	{
		gloox::Tag *tg1=s->findChild("x", "xmlns",
				"http://jabber.org/protocol/muc#user");
		assert(tg1);
		if (tg1->hasChild("item", nname))
		{
			gloox::Tag *tg2=tg1->findChild("item", nname);
			assert(tg2);
			res=tg2->findAttribute(nname);
		}
	}
	return QString::fromStdString(res);
}

int MucPlugin::getStatus(gloox::Stanza* s)
{
	std::string res;
	if (s->hasChild("x", "xmlns", "http://jabber.org/protocol/muc#user"))
	{
		gloox::Tag *tg1=s->findChild("x", "xmlns",
				"http://jabber.org/protocol/muc#user");
		if (!tg1)
			return -1;
		if (tg1->hasChild("status", "code"))
		{
			gloox::Tag *tg2=tg1->findChild("status", "code");
			if (!tg2)
				return -1;
			res=tg2->findAttribute("code");
		}
	}
	return QString::fromStdString(res).toInt();
}


QString MucPlugin::getReason(gloox::Stanza* s)
{
	std::string res;
	if (s->hasChild("x", "xmlns", "http://jabber.org/protocol/muc#user"))
	{
		gloox::Tag *tg1=s->findChild("x", "xmlns",
				"http://jabber.org/protocol/muc#user");
		if (!tg1)
			return "";
		if (tg1->hasChild("item"))
		{
			gloox::Tag *tg2=tg1->findChild("item");
			if (!tg2)
				return "";
			tg2 = tg2->findChild("reason");
			if (!tg2)
				return "";
			res = tg2->cdata();
		}
	}
	return QString::fromStdString(res);
}

bool MucPlugin::canHandleIq(gloox::Stanza* /* s */)
{
	/*	std::cout << "!!!!! " << s->xml() << std::endl;
	 Conference *conf = getConf(s);
	 if (!conf) return false;
	 return true;*/
	return false;
}

bool MucPlugin::onIq(gloox::Stanza* s)
{
	Conference *conf = getConf(s);
	if (!conf)
		return false;
	Nick *nick=getNick(s);
	if (!nick)
		return false;

	AsyncRequest* req=bot()->asyncRequests()->byStanza(s);
	if (!req)
		return false;

	QString xmlns=QString::fromStdString(s->xmlns());
	if (s->subtype()!=gloox::StanzaIqResult || xmlns!="jabber:iq:version")
		return false;

	gloox::Tag* query=s->findChild("query", "xmlns", xmlns.toStdString());
	nick->setVersionName(QString::null);
	nick->setVersionClient(QString::null);
	nick->setVersionOs(QString::null);
	if (query)
	{
		gloox::Tag* t;
		t=query->findChild("name");
		if (t)
			nick->setVersionName(QString::fromStdString(t->cdata()));
		t=query->findChild("version");
		if (t)
			nick->setVersionClient(QString::fromStdString(t->cdata()));
		t=query->findChild("os");
		if (t)
			nick->setVersionOs(QString::fromStdString(t->cdata()));
	}
	delete req;
	bot()->asyncRequests()->removeAll(req);
	nick->setVersionStored(true);

	JidStat *stat = nick->jidStat();
	if (stat)
	{
		stat->setVersion(nick->versionName(), nick->versionClient(),
				nick->versionOs());
	}

	checkMember(0L, conf, nick, AListItem::MatcherVersion);
	return true;
}

QString MucPlugin::getIqError(gloox::Stanza *s)
{
	std::string res;
	if (s->hasChild("error"))
	{
		gloox::Tag *tg1=s->findChild("error");
		assert(tg1);
		res=tg1->findAttribute("type") + " " + tg1->findAttribute("code");
	}
	return QString::fromStdString(res);
}

bool MucPlugin::autoLists(gloox::Stanza *s, MessageParser& parser)
{
	Conference* conf=getConf(s);
	QString arg=parser.nextToken().toUpper();
	QString arg2=parser.nextToken();

	QString nickName=QString::fromStdString(s->from().resource());

	if (arg=="AFIND")
	{
		QString answer;
		Nick *n=conf->nicks()->byName(arg2);
		if (!n)
		{
			reply(s, QString("Can't see \"%1\" here").arg(arg2));
			return true;
		}

		QList<AListItem*> itemList;
		itemList=aFind(conf->aban(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in aban list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		itemList=aFind(conf->akick(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in akick list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		itemList=aFind(conf->avisitor(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in avisitor list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		itemList=aFind(conf->aparticipant(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in aparticipant list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		itemList=aFind(conf->amoderator(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in amoderator list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		itemList=aFind(conf->acommand(), n, 0L, AListItem::MatcherAll,false);
		if (!itemList.isEmpty())
			answer+=QString("\"%1\" is in acmd list:\n%2\n").arg(arg2.toLower()).arg(AListItem::toStringAll(itemList));

		if (answer.endsWith("\n"))
			answer.remove(answer.length()-1, 1);
		if (answer.isEmpty())
			answer=QString("\"%1\" is not found in a-lists").arg(arg2.toLower());
		reply(s, answer);
		return true;
	}

	AList* alist;
	bool isAndCondition=false;

	alist=alistByName(conf, arg);
	if (!alist)
		isAndCondition=(arg.toUpper()=="AAND");

	if (!alist && !isAndCondition)
	{
		reply(s, QString("Try \"!muc help\""));
		return TRUE;
	}
	arg=arg.toLower();
	//parser.back();
	arg2=arg2.toUpper();
	bool isRemoving=false;
	if (arg2=="HELP" || arg2=="")
	{
		reply(s, QString("\"%1\" commands: help, show, count, clear,"
				" del <item>, exp <regexp>, <jid>").arg(arg));
		return TRUE;
	}
	if (arg2=="SHOW")
	{
		if (!alist)
		{
			reply(s, "No alist specified");
			return true;
		}
		alist->removeExpired();
		if (!alist->count())
			reply(s, QString("\"%1\" list is empty").arg(arg));
		else
		{
			reply(s, QString("\"%1\" list:\n%2").arg(arg).arg(alist->toString()));
		}
		return TRUE;
	}
	if (arg2=="COUNT")
	{
		if (!alist)
		{
			reply(s, "No alist specified");
			return true;
		}
		reply(s, QString("Currntly list \"%1\" contains %2 items").arg(arg).arg(alist->count()));
		return true;
	}
	if (arg2=="CLEAR")
	{
		if (!alist)
		{
			reply(s, "No alist specified");
			return true;
		}
		alist->removeItems();
		reply(s, "Cleared");
		return TRUE;
	}
	if (arg2=="DEL")
	{
		bool ok;
		QString itemNum=parser.nextToken();
		int idx=itemNum.toInt(&ok);
		int n;
		if (ok)
		{
			if (!alist)
			{
				reply(s, "No alist specified");
				return true;
			}
			if (idx>alist->count() || idx<1)
			{
				reply(s, "List index out of bounds");
				return true;
			}
			alist->removeAt(idx-1);
			n=1;
			reply(s, QString("%1 items removed").arg(n));
			return TRUE;
		}
		else
		{
			parser.back(1);
			arg2=parser.nextToken().toUpper();
			isRemoving=true;
		}
	}

	int howLong=0;
	if (arg2.startsWith('/'))
	{
		QString arg2u=arg2.toUpper();
		int k=0;
		if (arg2u.endsWith("D"))
			k=60*24;
		else if (arg2u.endsWith("H"))
			k=60;
		else if (arg2u.endsWith("M"))
			k=1;
		if (k!=0)
		{
			arg2.remove(arg2.length()-1, 1);
			qDebug() << arg2;
		}
		arg2.remove(0, 1);
		if (k==0)
			k=1;
		bool ok;
		int dt=arg2.toInt(&ok);
		if (!ok)
		{
			reply(s, "time is invalid. Should be NUM[d|h|m]");
			return true;
		}
		howLong=dt*k;
		arg2=parser.nextToken().toUpper();
	}
	parser.back();
	AListItem item;
	int res=parseAListItem(s, parser, item);
	if (res<0)
		return true;
	if (isAndCondition)
	{
		// Multiple 'AND' conditions
		AListItem* child=new AListItem();
		item.setChild(child);
		for (;;)
		{
			QString alistType=parser.nextToken();
			alist=alistByName(conf, alistType);
			if (alist==0)
			{
				isAndCondition=(alistType.toUpper()=="AAND");
				if (!isAndCondition)
				{
					reply(s, QString("Unable to parse alist definition at \"%1\"").arg(alistType));
					return true;
				}
			}
			int childRes=parseAListItem(s, parser, *child);
			if (childRes<0) return true;
			if (!alist && isAndCondition)
			{
				AListItem* newChild=new AListItem();
				child->setChild(newChild);
				child=newChild;
			}
			else
				break;
		}
	}

	if (howLong)
	{
		QDateTime t=QDateTime::currentDateTime().addSecs(howLong*60);
		item.setExpire(t);
	}

	if (!alist)
	{
		reply(s, "No alist specified");
		return true;
	}

	int existsIdx=alist->indexOfSameCondition(item);
	if (existsIdx>=0)
	{
		alist->removeAt(existsIdx);
		if (isRemoving)
		{
			reply(s, "Removed");
			return true;
		}
	}
	else
	{
		if (isRemoving)
		{
			reply(s,"Can't find item to remove");
			return true;
		}
	}
	QString reason=parser.joinBody().trimmed();
	if (!reason.isEmpty())
	{
		item.setReason(reason);
	}
	alist->append(item);

	// Remove all JID's from AListItem to avoid displaying it to chatroom
	AListItem* ptr=&item;
	while (ptr)
	{
		if (ptr->matcherType()==AListItem::MatcherJid)
		{
			QString jid=ptr->value();
			bool found=false;
			for (NickList::iterator it=conf->nicks()->begin(); it!=conf->nicks()->end(); ++it)
			{
				Nick *tmpNick=*it;
				if (tmpNick->jidStr().section('/',0,-2)==jid)
				{
					jid=QString("%1@").arg(tmpNick->nick());
					found=true;
					break;
				}
			}
			if (!found)
			{
				jid=jid.section('@',0,0)+"@";
			}
			ptr->setValue(jid);
		}
		ptr=ptr->child();
	}

	QString response((existsIdx>=0) ? "Updated" : "Added");
	response+=": ";
	response+=item.toString();
	reply(s, response);
	recheckJIDs(conf);
	return true;
}

AList* MucPlugin::alistByName(Conference* conf, const QString& name)
{
	AList* alist=0l;
	QString arg=name.toUpper();

	if (arg=="ABAN")
		alist=conf->aban();
	if (arg=="AKICK")
		alist=conf->akick();
	if (arg=="AVISITOR")
		alist=conf->avisitor();
	if (arg=="AMODERATOR")
		alist=conf->amoderator();
	if (arg=="APARTICIPANT")
		alist=conf->aparticipant();
	if (arg=="ACMD")
		alist=conf->acommand();
	return alist;
}

/**
 * @return -1 -- incorrect input, 0 -- alist finished, 1 -- more conditions available
 */
int MucPlugin::parseAListItem(gloox::Stanza* s, MessageParser& parser, AListItem& item)
{
	item.setMatcherType(AListItem::MatcherJid);
	item.setTestType(AListItem::TestExact);

	QString arg2=parser.nextToken().toUpper();

	if (arg2=="!")
	{
		item.setInvert(true);
		arg2=parser.nextToken().toUpper();
	}

	if (arg2=="NICK" || arg2=="BODY" || arg2=="RES"
		|| arg2=="VERSION" || arg2=="VERSION.NAME"
		|| arg2=="VERSION.CLIENT" || arg2=="VERSION.OS"
		|| arg2=="VCARD.PHOTOSIZE" || arg2=="AGE" || arg2=="ROLE")
	{
		if (arg2=="NICK")
			item.setMatcherType(AListItem::MatcherNick);
		else if (arg2=="BODY")
			item.setMatcherType(AListItem::MatcherBody);
		else if (arg2=="RES")
			item.setMatcherType(AListItem::MatcherResource);
		else if (arg2=="VERSION")
			item.setMatcherType(AListItem::MatcherVersion);
		else if (arg2=="VERSION.NAME")
			item.setMatcherType(AListItem::MatcherVersionName);
		else if (arg2=="VERSION.CLIENT")
			item.setMatcherType(AListItem::MatcherVersionClient);
		else if (arg2=="VERSION.OS")
			item.setMatcherType(AListItem::MatcherVersionOs);
		else if (arg2=="VCARD.PHOTOSIZE")
			item.setMatcherType(AListItem::MatcherVCardPhotoSize);
		else if (arg2=="AGE")
			item.setMatcherType(AListItem::MatcherAge);
		else if (arg2=="ROLE")
			item.setMatcherType(AListItem::MatcherRole);

		arg2=parser.nextToken().toUpper();
	}
	else if (arg2=="JID")
	{
		item.setMatcherType(AListItem::MatcherJid);
		arg2=parser.nextToken().toUpper();
	}

	if (arg2=="EXP")
	{
		QString expStr=parser.nextToken();
		QRegExp exp(expStr);
		if (exp.isValid())
		{
			parser.back();
			arg2=parser.nextToken().toUpper();
			item.setTestType(AListItem::TestRegExp);
		}
		else
		{
			reply(s, "QRegExp is not valid");
			return -1;
		}
	}
	else if (arg2=="SUB")
	{
		item.setTestType(AListItem::TestSubstring);
		arg2=parser.nextToken().toUpper();
	}
	else if (arg2==">")
	{
		item.setTestType(AListItem::TestGreater);
		arg2=parser.nextToken().toUpper();
	}
	else if (arg2=="<")
	{
		item.setTestType(AListItem::TestLesser);
		arg2=parser.nextToken().toUpper();
	}
	else if (item.matcherType()==AListItem::MatcherJid)
	{
		parser.back(1);
		QString args=parser.nextToken();
		QRegExp exp("[^@ ]*@[^ ]*");
		exp.setMinimal(FALSE);
		int ps=exp.indexIn(args);
		if (ps==0 && exp.matchedLength()==args.length())
		{
			// All ok
		}
		else
		{
			qDebug() << args;
			Conference *conf=getConf(s);
			Nick *n=conf->nicks()->byName(args);
			if (n && !n->jidStr().isEmpty())
				arg2=n->jidStr().section('/', 0, 0);
			else
			{
				reply(s, "JID or nick is not valid: "+args);
				return -1;
			}
		}
	}

	arg2=arg2.toLower();
	item.setValue(arg2);
	return 0;
}

// Stanza can be null here
const QList<AListItem*> MucPlugin::aFind(AList* list, Nick* nick, gloox::Stanza* s, AListItem::MatcherType matcher, bool onlyFirst)
{
	QList<AListItem*> resultList;
	int cnt=list->count();
	QString line;
	QString lJid=nick->jidStr().toLower().section('/', 0, 0);
	QString lResource=nick->jidStr().toLower().section('/', 1);
	QString lNick=nick->nick().toLower();
	QString lBody;
	if (s)
		lBody=QString::fromStdString(s->body()).toLower();
	QString version;
	if (nick->isVersionStored())
	{
		version=nick->versionName();
		if (!nick->versionClient().isEmpty())
			version+=" "+nick->versionClient();
		if (!nick->versionOs().isEmpty())
			version+=" // "+nick->versionOs();
		version=version.trimmed().toLower();
	}
	QString vcardPhotoSize=QString::number(nick->vcardPhotoSize());
	QString age("?");
	QString role("0");
	if (nick->jid())
	{
		age=QString::number(nick->jid()->created().secsTo(QDateTime::currentDateTime()));
		role=QString::number(getRole(nick->conference()->name()+"/"+nick->nick()));
	}

	bool isPresence=!s || (s->type()==gloox::StanzaPresence);

	for (int i=0; i<cnt; i++)
	{
		AListItem* topItem=list->at(i);
		AListItem* item=topItem;
		bool multiMatch=(topItem->child()!=0);
		bool chainMatches=true;

		if (multiMatch && !lBody.isEmpty() && !item->isBodyDepends())
			continue;

		while (item)
		{
			bool partMatches=false;
			bool processEmpty=false;
			if (!multiMatch && matcher!=AListItem::MatcherUnknown && matcher!=AListItem::MatcherAll &&
					item->matcherType()!=matcher)
			{
				if (matcher==AListItem::MatcherVersion)
				{
					if (!item->isSubVersionMatcher())
						break;
				}
				else
					break;
			}

			if (!multiMatch && item->matcherType()==AListItem::MatcherVCardPhotoSize
					&& matcher!=item->matcherType() && matcher!=AListItem::MatcherAll)
				break;

			QString testValue;
			if (!multiMatch && !isPresence && item->isSubPresenceDepends())
				break;

			if (item->isSubVersionMatcher())
			{
				if (!multiMatch && matcher!=AListItem::MatcherVersion && matcher!=AListItem::MatcherAll)
					break;
				if (!nick || !nick->isVersionStored())
					break;
				processEmpty=true;
			}

			bool shouldBreak=false;
			switch (item->matcherType())
			{
				case AListItem::MatcherUnknown: break;
				case AListItem::MatcherNick: testValue=lNick; break;
				case AListItem::MatcherJid: testValue=lJid; break;
				case AListItem::MatcherResource: testValue=lResource; break;
				case AListItem::MatcherBody: testValue=lBody; break;
				case AListItem::MatcherVersion: testValue=version; break;
				case AListItem::MatcherVersionName: testValue=nick->versionName().toLower(); break;
				case AListItem::MatcherVersionClient: testValue=nick->versionClient().toLower(); break;
				case AListItem::MatcherVersionOs: testValue=nick->versionOs().toLower(); break;
				case AListItem::MatcherVCardPhotoSize: testValue=vcardPhotoSize; break;
				case AListItem::MatcherAge: testValue=age; break;
				case AListItem::MatcherRole: testValue=role; break;
				default: shouldBreak=true; break;
			}
			if (shouldBreak)
				break;

			if (!processEmpty && testValue.isEmpty())
				break;

			switch (item->testType())
			{
				case AListItem::TestUnknown: break;
				case AListItem::TestRegExp:
				{
					QRegExp exp(item->value());
					exp.setMinimal(FALSE);
					exp.setCaseSensitivity(Qt::CaseInsensitive);
					if (item->isInvert() ^ exp.exactMatch(testValue))
						partMatches=true;
					break;
				}
				case AListItem::TestExact:
				{
					if (item->isInvert() ^ testValue==item->value())
						partMatches=true;
					break;
				}
				case AListItem::TestSubstring:
				{
					if (item->isInvert() ^ (testValue.toLower().indexOf(item->value().toLower())>=0))
						partMatches=true;
					break;
				}
				case AListItem::TestGreater:
				case AListItem::TestLesser:
				{
					bool ok1=true;
					bool ok2=true;
					int v1=testValue.toInt(&ok1);
					int v2=item->value().toInt(&ok2);
					bool cmpResult=false;
					if (ok1 && ok2)
					{
						//Compare as numbers
						if (item->testType()==AListItem::TestGreater)
							cmpResult=(v1>v2);
						else
							cmpResult=(v1<v2);
					}
					else
					{
						//Compare strings
						if (item->testType()==AListItem::TestGreater)
							cmpResult=(testValue > item->value());
						else
							cmpResult=(testValue < item->value());
					}
					if (item->isInvert() ^ cmpResult)
						partMatches=true;
				}
			}
			chainMatches=chainMatches && partMatches;
			if (!chainMatches)
				break;
			item=item->child();
		}
		if (chainMatches && !item)
		{
			resultList.append(topItem);
			if (onlyFirst)
				break;
		}
	}
	if (!resultList.isEmpty() && !nick->conference()->alistTraceList()->isEmpty())
	{
		QString tracedMsg=QString("Nick \"").append(nick->nick()).append("\" (").append(nick->jidStr()).append(")")
			.append(" is matched followed items of a").append(list->name()).append(" list:\n");
		tracedMsg+=AListItem::toStringAll(resultList);
		for (QStringList::iterator it=nick->conference()->alistTraceList()->begin();
			it!=nick->conference()->alistTraceList()->end(); ++it)
		{
			QString dst=*it;
			gloox::Stanza* s=gloox::Stanza::createMessageStanza(gloox::JID(dst.toStdString()), tracedMsg.toStdString());
			bot()->client()->send(s);
		}
	}
	return resultList;
}

void MucPlugin::checkMember(gloox::Stanza* s, Conference*c, Nick* n, AListItem::MatcherType matcher)
{
	if (!n || !c)
		return;

	// Don't process our own presences
	if (c && n->nick()==c->nick())
		return;

	// We should not parse own messages, since this can create loop
	if (s && (s->from() == s->to() || isMyMessage(s)))
		s=0l;

	QList<AListItem*> itemList;

	Nick::Affiliation aff = n->affiliationValue();
	Nick::Role role = n->roleValue();

	itemList=aFind(c->acommand(), n, s, matcher, false);
	if (!itemList.isEmpty())
	{
		for (QList<AListItem*>::const_iterator it=itemList.begin(); it!=itemList.end();  ++it)
		{
			AListItem* item=*it;

			if (c->history())
				c->history()->log(n, NULL, ActionAListCommand, item->toString(), false,
						QString::number(item->id()));

			QString action=item->reason();
			if (action.isEmpty())
				return;
			action=expandMacro(s,c,n,action,item);
			QString from(c->name()+"/"+c->nick());
			QString type="groupchat";
			if (s)
			{
				QString t1=QString::fromStdString(s->findAttribute("type"));
				if (!t1.isEmpty())
					type=t1;
				if (type != "groupchat")
					from=QString::fromStdString(s->from().full());
			}
			gloox::Stanza* st=gloox::Stanza::createMessageStanza(from.toStdString(),
					action.toStdString());
			st->addAttribute("from", from.toStdString());
			st->addAttribute("type", type.toStdString());
			st->finalize();
			bot()->processMessage(st, QStringList("acmd"));
			delete st;
		}
	}

	if (aff < Nick::AffiliationMember || c->configurator()->isApplyAlistsToMembers())
	{
		if (aff < Nick::AffiliationAdmin)
		{
			itemList=aFind(c->aban(), n, s, matcher,true);
			if (!itemList.isEmpty())
			{
				AListItem* item=itemList.first();
				if (s && warnImOwner(s))
					return;

				if (c->history())
					c->history()->log(n, NULL, ActionAListBan, item->toString(), false,
							QString::number(item->id()));

				QString reason=item->reason();
				if (reason.isEmpty())
					reason=DataStorage::instance()->getString("str/ban_reason");
				setAffiliation(c, n->jidStr(), "outcast",reason);
				return;
			}
		}

		if (role < Nick::RoleModerator)
		{
			itemList=aFind(c->akick(), n, s, matcher, true);
			if (!itemList.isEmpty())
			{
				AListItem* item=itemList.first();

				if (c->history())
					c->history()->log(n, NULL, ActionAListKick, item->toString(), false,
							QString::number(item->id()));

				QString reason=item->reason();
				if (reason.isEmpty())
					reason=DataStorage::instance()->getString("str/kick_reason");
				setRole(c, n, "none", reason);
				return;
			}
		}

		if (aff < Nick::AffiliationAdmin && role != Nick::RoleVisitor)
		{
			itemList=aFind(c->avisitor(), n, s, matcher, true);
			if (!itemList.isEmpty())
			{
				AListItem* item=itemList.first();

				if (c->history())
					c->history()->log(n, NULL, ActionAListVisitor, item->toString(), false,
							QString::number(item->id()));

				QString reason=item->reason();
				if (reason.isEmpty())
					reason=DataStorage::instance()->getString("str/visitor_reason");
				setRole(c, n, "visitor", reason);
				return;
			}
		}
	}

	if (aff < Nick::AffiliationAdmin && role != Nick::RoleParticipant)
	{
		itemList=aFind(c->aparticipant(), n, s, matcher, true);
		if (!itemList.isEmpty())
		{
			AListItem* item=itemList.first();

			if (c->history())
				c->history()->log(n, NULL, ActionAListParticipant, item->toString(), false,
						QString::number(item->id()));

			QString reason=item->reason();
			if (reason.isEmpty())
				reason=DataStorage::instance()->getString("str/participant_reason");
			setRole(c, n, "participant", reason);
			return;
		}
	}

	if (role != Nick::RoleModerator)
	{
		itemList=aFind(c->amoderator(), n, s, matcher, true);
		if (!itemList.isEmpty())
		{
			AListItem* item=itemList.first();

			if (c->history())
				c->history()->log(n, NULL, ActionAListModerator, item->toString(), false,
						QString::number(item->id()));


			QString reason=item->reason();
			if (reason.isEmpty())
				reason=DataStorage::instance()->getString("str/moderator_reason");
			setRole(c, n, "moderator", reason);
			return;
		}
	}
}

void MucPlugin::recheckJIDs(Conference *c)
{
	c->removeExpired();
	int cnt=c->nicks()->count();
	for (int i=0; i<cnt; i++)
	{
		Nick *n=c->nicks()->at(i);
		assert(n);
		checkMember(0L, c, n);
	}
}

// Storage provider
StorageKey MucPlugin::getStorage(gloox::Stanza *s)
{
	Conference* conf=getConf(s);
	if (!conf)
		return StorageKey();

	return StorageKey(pluginId, conf->id());
}

AbstractConfigurator* MucPlugin::getConfigurator(gloox::Stanza* s)
{
	Conference* conf=getConf(s);
	if (!conf)
		return 0;
	if (getRole(s)<ROLE_ADMIN)
		return 0;

	return conf->configurator();
}

QString MucPlugin::getJID(gloox::Stanza*s, const QString& n, bool realJid)
{
	Nick* nick=getNick(s, n);
	if (!nick)
		return QString::null;
	Conference *conf=nick->conference();
	if (!conf)
		return QString::null;
	if (realJid && !nick->jidStr().isEmpty())
		return nick->jidStr();
	return QString("%1/%2").arg(conf->name()).arg(nick->nick());
}

QString MucPlugin::getBotJID(gloox::Stanza* s)
{
	Conference* conf=getConf(s);
	if (!conf)
		return QString::null;

	return QString("%1/%2").arg(conf->name()).arg(conf->nick());
}

QString MucPlugin::JIDtoNick(const QString& jid)
{
	QString c=jid.section('/', 0, 0);
	QString n=jid.section('/', 1);
	Conference *conf=conferences.byName(c);
	if (conf)
		return n;
	return QString::null;
}

void MucPlugin::sendMessage(Conference *conf, const QString& msg)
{
	if (!conf)
		return;
	QString jid=conf->name();

	gloox::Stanza *st=gloox::Stanza::createMessageStanza(
			gloox::JID(jid.toStdString()), msg.toStdString());
	st->addAttribute("type", "groupchat");
	bot()->client()->send(st);
}

void MucPlugin::onQuit(const QString& reason)
{
}

QRegExp MucPlugin::getConfExp(const QString& from)
{
	QString expStr=from.section('/', 0, 0);
	expStr.replace(".", "\\.");
	expStr.replace("@", "\\@");
	QRegExp exp(QString("^%1/.*").arg(expStr));
	exp.setCaseSensitivity(Qt::CaseInsensitive);
	return exp;
}

QString MucPlugin::affiliationByCommand(const QString& cmd)
{
	if (cmd=="BAN" || cmd=="BANJID")
		return "outcast";
	if (cmd=="ADMIN")
		return "admin";
	if (cmd=="MEMBER")
		return "member";
	if (cmd=="OWNER")
		return "owner";
	return "none";
}

bool MucPlugin::warnImOwner(gloox::Stanza* s)
{
	QString myNick=getMyNick(s);
	Nick* nick=getNick(s, myNick);
	if (!nick)
		return false;

	if (nick->affiliation().toUpper() == "OWNER")
	{
		reply(s,
				"I'm conference owner. Affiliation editor is disabled for security purposes");
		return true;
	}

	return false;
}

QString MucPlugin::expandMacro(gloox::Stanza* s, Conference*c, Nick* n, const QString& str, const AListItem* item)
{
	QString msg=str;
	if (c)
	{
		msg.replace("${CONFERENCE}",c->name());
		msg.replace("${MYNICK}",c->nick());
	}
	if (n)
	{
		msg.replace("${JID}", n->jidStr());
		msg.replace("${NICK}",n->nick());
	}
	if (s)
	{
		QString body=QString::fromStdString(s->body());
		msg.replace("${BODY}", body);

		if (item)
		{
			QString value=item->value();

			if (!value.isEmpty() && msg.indexOf("${BODYARG}")>=0)
			{
				if (body.startsWith(value))
				{
					body.remove(0,value.length());
				}
				else
				{
					int idx=body.indexOf(value, Qt::CaseSensitive);
					if (idx>=0)
					{
						body.remove(idx,value.length());
						body.replace("  "," ");
					}
				}
				msg.replace("${BODYARG}", body.trimmed());
			}
		}
	}
	return msg;
}

void MucPlugin::requestVCard(gloox::Stanza* s, Conference* conf, Nick* nick)
{
	if (nick->jidStr().isEmpty())
		return;

	QString jid=conf->name()+"/"+nick->nick();
	QString vcardId=bot()->client()->fetchVCard(jid);
	gloox::Stanza *sf=new gloox::Stanza(s);
	sf->addAttribute("id", QString("muc_vcard_%1").arg(vcardId).toStdString());
	sf->addAttribute("gluxi_jid", nick->jidStr().toStdString());
	AsyncRequest *req=new AsyncRequest(-1, this, sf, 3600);
	req->setName("muc::vcard: "+jid);
	bot()->asyncRequests()->append(req);
}

bool MucPlugin::onVCard(const VCardWrapper& vcardWrapper)
{
	const gloox::JID jid=vcardWrapper.jid();
	gloox::VCard vcard=gloox::VCard(vcardWrapper.vcard());

	qDebug() << "Got vcard: "+vcardWrapper.id();
	QString jidStr=QString::fromStdString(jid.full());
	QString reqId=QString("muc_vcard_%1").arg(vcardWrapper.id());
	AsyncRequest* req=bot()->asyncRequests()->byStanzaId(reqId);
	if (req==0l)
	{
		return false;
	}

	gloox::Stanza* src=req->stanza();
	Conference* conf=getConf(src);
	Nick* nick=getNick(src);
	QString stored_jid=QString::fromStdString(src->findAttribute("gluxi_jid"));

	if (conf && nick && stored_jid==nick->jidStr())
	{
		if (vcardWrapper.isEmpty() || vcardWrapper.vcardStr().isEmpty())
		{
			nick->setVCardPhotoSize(0);
			if (conf->configurator()->isDevoiceNoVCard())
			{
				// Devoice only if user was not already devoiced (for example by alists)
				if (nick->role()!="visitor")
				{
					reply(src, conf->configurator()->devoiceNoVCardReason(),true, true);
					setRole(conf, nick, "visitor", conf->configurator()->devoiceNoVCardReason());
					nick->setDevoicedNoVCard(true);
				}
			}
		}
		else
		{
			nick->setVCardPhotoSize(vcardWrapper.photoSize());
			if (nick->isDevoicedNoVCard() && conf->configurator()->isDevoiceNoVCard() && nick->role().toUpper()=="VISITOR")
			{
				setRole(conf, nick, "participant","");
				nick->setDevoicedNoVCard(false);
			}
		}
		checkMember(0, conf, nick, AListItem::MatcherVCardPhotoSize);
	}
	delete req;
	bot()->asyncRequests()->removeAll(req);
	return true;
}

void MucPlugin::sltAutoLeaveTimerTimeout()
{
	qDebug() << "Checking for died conferences";

	QStringList conferencesToLeave=Conference::autoLeaveList();
	if (conferencesToLeave.isEmpty())
		return;
	QStringList report;
	for (QStringList::iterator it=conferencesToLeave.begin(); it!=conferencesToLeave.end(); ++it)
	{
		QString value=(*it);
		int cnt=value.section(' ',0,0).toInt();
		QString name=value.section(' ',1);
		report.append(QString("%1: %2 jids").arg(name).arg(cnt));
		leave(name);
	}
	gloox::JID ownerJid(DataStorage::instance()->getStdString("access/owner"));
	gloox::Stanza *outgoing=gloox::Stanza::createMessageStanza(ownerJid,
		QString("I'm leaving followed died conferneces:\n%1").arg(report.join("\n")).toStdString());
	bot()->client()->send(outgoing);
}

void MucPlugin::requestVersion(gloox::Stanza* s, Conference* conf, Nick* nick)
{
	QString jid=conf->name()+"/"+nick->nick();
	std::string id=bot()->client()->getID();

	gloox::Stanza *st=gloox::Stanza::createIqStanza(
			gloox::JID(jid.toStdString()), id, gloox::StanzaIqGet,
			"jabber:iq:version");

	gloox::Stanza *sf=new gloox::Stanza(s);
	sf->addAttribute("id", id);

	int timeout=conf->configurator()->queryVersionTimeout();
	NickAsyncRequest *req=new NickAsyncRequest(-1, this, sf,
			timeout==0 ? 3600: timeout, timeout!=0);
	connect(req, SIGNAL(onTimeout(AsyncRequest*)), SLOT(sltVersionQueryTimeout(AsyncRequest*)));
	req->setName("muc::version: "+jid);
	req->setNick(nick->nick());
	req->setJid(nick->jidStr());
	req->setConference(conf->name());
	bot()->asyncRequests()->append(req);
	bot()->client()->send(st);
	return;
}

int MucPlugin::getRoleForNick(Conference* conf, Nick* nick)
{
	if (!conf || !nick)
		return 0;
	QString jid1=nick->jidStr();
	QString jid2=conf->name()+"/"+nick->nick();
	int role1=bot()->roles()->get(jid1);
	int role2=bot()->roles()->get(jid2);
	if (role1<0)
		return role1;
	if (role2<0)
		return role2;
	return (role1>role2) ? role1: role2;
}

void MucPlugin::sltVersionQueryTimeout(AsyncRequest* req)
{
	qDebug() << "Version query timeout: " << req->name();
	if (!req->name().startsWith("muc::version"))
	{
		qDebug() << "[BUG] Incorrect request";
	}
	NickAsyncRequest* nickReq=static_cast<NickAsyncRequest*>(req);
	Conference *conf=conferences.byName(nickReq->conference());
	if (!conf)
		return;
	Nick* nick=conf->nicks()->byName(nickReq->nick());
	if (!nick)
		return;
	if (nick->jidStr()!=nickReq->jid())
		return;
	nick->setVersionStored(true);
	checkMember(0L, conf, nick, AListItem::MatcherVersion);
}


bool MucPlugin::ageLessThan(const Nick* nick1, const Nick* nick2)
{
	QDateTime date1=QDateTime::currentDateTime();
	QDateTime date2=date1;
	if (nick1->jid())
		date1=nick1->jid()->created();
	if (nick2->jid())
			date2=nick2->jid()->created();
	return date1 < date2;
}

QString MucPlugin::invite(gloox::Stanza* s, const QString& n, const QString& reason)
{
	Conference* conf = getConf(s);
	if(!conf) {
		return "You are not in conference!";
	}
	QStringList nicks = n.split(' ');
	QStringList jids;
	Nick *nick = 0;
	for( int ndx = 0; ndx < nicks.size(); ndx++ ) {
		if(isBareJidValid(nicks.at(ndx))) {
			jids.append(nicks.at(ndx));
		} else {
			nick = conf->nicks()->byName(nicks.at(ndx));
			if(nick) {
				jids.append(nick->jid()->jid());
			}
			else
			{
				jids = Nick::nickToJids(conf, nicks.at(ndx), true);
			}
		}
	}
	if( reason.length() > 255 )
			reason.left(252).append("...");
	gloox::Stanza* st = invite(conf, jids, reason);

	if( !st ) { /// TODO: Multi-invites.
		QString str = "Can't resolve: \"%1\"'s JID";
		str = (nicks.count() > 1) ? str + "s.": str + ".";
		return str.arg(nicks.join("\n"));
	}

	bot()->client()->send(st);

	return "Invitation sent.";
}

gloox::Stanza*
MucPlugin::invite(Conference *conf, const QStringList& jids, const QString& reason, const QString& pass)
{
	if(!conf) return 0;
	bool validJID = false;
	gloox::Tag *m = new gloox::Tag( "message" );
	m->addAttribute( "to", conf->name().toStdString() );
	gloox::Tag *x = new gloox::Tag( m, "x" );
	x->addAttribute( "xmlns", "http://jabber.org/protocol/muc#user" );

	for( int ndx = 0; ndx < jids.size(); ndx++ ) {
		if(isBareJidValid(jids.at(ndx))) {
			validJID = true;
			gloox::Tag *i = new gloox::Tag( x, "invite" );
			i->addAttribute( "to", jids.at(ndx).toStdString() );
			qDebug() << "JID: " << jids.at(ndx);
			if(!reason.isEmpty())
				new gloox::Tag( i, "reason",
					reason.toStdString() );
		}
	}
	if( !validJID ) {
		return 0;
	}
	if(!pass.isEmpty()) {
		new gloox::Tag( x, "password", pass.toStdString() );
	}
	return (new gloox::Stanza(m));
}
