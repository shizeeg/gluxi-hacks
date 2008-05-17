#include "mucplugin.h"
#include "conference.h"
#include "nicklist.h"
#include "alist.h"
#include "alistitem.h"
#include "jid.h"

#include "base/common.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/datastorage.h"
#include "base/rolelist.h"
#include "base/messageparser.h"

#include <QtDebug>
#include <QRegExp>

#include <iostream>

#include <gloox/stanza.h>
#include <assert.h>

MucPlugin::MucPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "WHEREAMI" << "NICK" << "IDLE" << "KNOWN" << "JOIN" << "LEAVE"
			<< "KICK" << "VISITOR" << "PARTICIPANT" << "MODERATOR" << "BAN"
			<< "BANJID" << "UNBAN" << "NONE" << "MEMBER" << "ADMIN" << "OWNER";
	commands << "ABAN" << "AKICK" << "AVISITOR" << "AMODERATOR" << "AFIND"
			<< "SEEN" << "CLIENTS" << "SETNICK";
	commands << "HERE";
	pluginId=1;
	lazyOffline=DataStorage::instance()->getInt("muc/lazyoffline");
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
	qDebug() << "MucPlugin::canHandlePresence() " << fromFull << " = " << ret;
	return ret;
}

bool MucPlugin::isMyMessage(gloox::Stanza*s)
{
	QString jid=QString::fromStdString(s->from().bare());
	Conference* conf=conferences.byName(jid);
	if (!conf)
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

bool MucPlugin::canHandleMessage(gloox::Stanza* s)
{
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
	if (n->nick()==conf->nick())
	{
		qDebug() << "MUC: Self message ignored";
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
				<< "[MUC] Got type='error' in onPresence. Looks like we can't join conference";
		int idx=confInProgress.indexOf(getConfExp(confFull));
		if (idx>=0)
			confInProgress.removeAt(idx);
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
		qDebug() << conf->name();
		conferences.append(conf);
		qDebug() << confInProgress;
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

	if (!n)
	{
		qDebug() << "MUC::handlePresence: new Nick";
		if (role=="none" || type=="unavailable")
			return;
		n=new Nick(conf, nick,getItem(s,"jid"));
		conf->nicks()->append(n);
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
				conf->setNick(newNick);
				conf->nicks()->remove(n);
			}
			else
			{
				qDebug() <<"!!! I'm kicked/banned";
				conferences.remove(conf);
			}
		}
		else
		{
			qDebug() << "!!!!!! Removing nick";
			conf->nicks()->remove(n);
		}
		bot()->roles()->remove(QString::fromStdString(s->from().full()));
	}
	else
	{
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
		checkMember(s, conf, n);
	}
}

bool MucPlugin::parseMessage(gloox::Stanza* s)
{
	if (isOfflineMessage(s))
		return true;

	MessageParser parser(s, getMyNick(s));
	QString msgPrefix=parser.nextToken().toUpper();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.nextToken();

	QString nickName=QString::fromStdString(s->from().resource());

	Conference* conf=getConf(s);

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

	checkMember(s, conf, nick);

	if (msgPrefix!=prefix() || !parser.isForMe())
	{
		myShouldIgnoreError=1;
		return false;
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
						QString("Nick \"%1\": Affiliation: %2; Role: %3; Registered: %4; Joined: %5; Idle: %6; Status: %7 (%8)")
						.arg(n->nick())
						.arg(n->affiliation())
						.arg(n->role())
						.arg(jidCreated)
						.arg(n->joined().toString(Qt::LocaleDate))
						.arg(secsToString(n->lastActivity().secsTo(QDateTime::currentDateTime())))
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

	if (cmd=="SEEN")
	{
		reply(s, conf->seen(arg));
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

		QString target;
		QString reason;
		int ps=arg.indexOf('|');
		if (ps>=0)
		{
			target=arg.section('|', 0, 0);
			reason=arg.section('|', 1);
		}
		else
		{
			target=arg.section(' ', 0, 0);
			reason=arg.section(' ', 1);
		}

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
			|| cmd=="AEDIT" || cmd=="AFIND")
	{
		if (!isFromConfAdmin(s))
			return true;
		parser.back(2);
		return autoLists(s, parser);
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

	return false;
}

Conference* MucPlugin::getConf(gloox::Stanza* s)
{
	QString confName=QString::fromStdString(s->from().bare());
	Conference* conf=conferences.byName(confName);
	if (!conf)
	{
		qDebug() << "!!!!!!!!! Conf not found";
		return 0;
	}
	return conf;
}

Nick* MucPlugin::getNick(gloox::Stanza* s, const QString& nn)
{
	Conference *conf=getConf(s);
	if (!conf)
		return 0;

	QString nickName;
	if (nn.isEmpty())
		nickName=QString::fromStdString(s->from().resource());
	else
		nickName=nn;

	qDebug() << "[GETNICK] " << nickName;

	Nick *nick=conf->nicks()->byName(nickName);
	return nick;
}

bool MucPlugin::isFromConfModerator(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().full());
	return (bot()->roles()->get(jid) >= ROLE_MODERATOR);
	/*	if (bot()->owners()->indexOf(jid)>=0)
	 return true
	 QString nm=QString::fromStdString(s->from().resource());
	 Nick *nick=getNick(s,nm);
	 if (!nick) return false;

	 if (nick->role()!="moderator")
	 {
	 reply(s,"Only conference moderator can do this.");
	 return false;
	 }
	 return true;
	 */

}

bool MucPlugin::isFromConfAdmin(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().full());
	return (bot()->roles()->get(jid) >= ROLE_ADMIN);
	/*
	 if (bot()->owners()->indexOf(jid)>=0)
	 return true;

	 QString nm=QString::fromStdString(s->from().resource());
	 Nick *nick=getNick(s,nm);
	 if (!nick) return false;


	 qDebug() << nick->affiliation();
	 if (!nick->affiliation().toUpper().startsWith("ADMIN") && nick->affiliation().toUpper() != "OWNER")
	 {
	 reply(s,"Only conference administrator can do this");
	 return false;
	 }
	 return true;
	 */
}

bool MucPlugin::isFromConfOwner(gloox::Stanza* s)
{
	/*	QString jid=QString::fromStdString(s->from().bare());
	 QString nm=QString::fromStdString(s->from().resource());
	 Nick *nick=getNick(s,nm);
	 if (!nick) return false;
	 */
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

void MucPlugin::join(const QString& name)
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
		conf->setAutoJoin(FALSE);

	/*	Conference *conf=new Conference(cname,cnick);
	 conferences.append(conf); */

	gloox::Stanza
			*st=
					gloox::Stanza::createPresenceStanza(gloox::JID(confName.toStdString()));
	st->addAttribute("type", "unavailable");

	std::cout << st->xml() << std::endl << std::endl;
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
	QString reason=getIqError(s);

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

	AList* alist=0;

	//	QString body="";
	//	int narg=0;
	//	QString args="";

	if (arg=="AFIND")
	{
		QString answer;
		Nick *n=conf->nicks()->byName(arg2);
		if (!n)
		{
			reply(s, QString("Can't see \"%1\" here").arg(arg2));
			return true;
		}

		if (aFind(conf->aban(), n, 0L))
			answer+=QString("\"%1\" is in aban list\n").arg(arg2.toLower());
		if (aFind(conf->akick(), n, 0L))
			answer+=QString("\"%1\" is in akick list\n").arg(arg2.toLower());
		if (aFind(conf->avisitor(), n, 0L))
			answer+=QString("\"%1\" is in avisitor list\n").arg(arg2.toLower());
		if (aFind(conf->amoderator(), n, 0L))
			answer+=QString("\"%1\" is in amoderator list\n").arg(arg2.toLower());
		if (answer.endsWith("\n"))
			answer.remove(answer.length()-1, 1);
		if (answer.isEmpty())
			answer=QString("\"%1\" is not found in a-lists").arg(arg2.toLower());
		reply(s, answer);
		return true;
	}

	if (arg=="ABAN")
		alist=conf->aban();
	if (arg=="AKICK")
		alist=conf->akick();
	if (arg=="AVISITOR")
		alist=conf->avisitor();
	if (arg=="AMODERATOR")
		alist=conf->amoderator();
	if (!alist)
	{
		reply(s, QString("Try \"!muc help\""));
		return TRUE;
	}
	arg=arg.toLower();
	//parser.back();
	arg2=arg2.toUpper();
	QString arg3=parser.nextToken();
	if (arg2=="HELP" || arg2=="")
	{
		reply(s, QString("\"%1\" commands: help, show, count, clear,"
				" del <item>, exp <regexp>, <jid>").arg(arg));
		return TRUE;
	}
	if (arg2=="SHOW")
	{
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
		reply(s, QString("Currntly list \"%1\" contains %2 items").arg(arg).arg(alist->count()));
		return true;
	}
	if (arg2=="CLEAR")
	{
		alist->removeItems();
		reply(s, "Cleared");
		return TRUE;
	}
	if (arg2=="DEL")
	{
		bool ok;
		int idx=arg3.toInt(&ok);
		int n;
		if (ok)
		{
			if (idx>alist->count() || idx<1)
			{
				reply(s, "List index out of bounds");
				return true;
			}
			alist->removeAt(idx-1);
			n=1;
		}
		else
		{
			reply(s, "Please specify number");
			return TRUE;
		}

		reply(s, QString("%1 items removed").arg(n));
		return TRUE;
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
		arg2=arg3.toUpper();
		arg3=parser.nextToken();
	}

	AListItem item;
	item.setMatcherType(AListItem::JID);

	if (arg2=="NICK" || arg2=="BODY")
	{
		if (arg2=="NICK")
			item.setMatcherType(AListItem::NICK);
		else if (arg2=="BODY")
			item.setMatcherType(AListItem::BODY);

		arg2=arg3.toUpper();
		arg3=parser.nextToken();
	}
	else if (arg2=="JID")
	{
		item.setMatcherType(AListItem::JID);
		arg2=arg3.toUpper();
		arg3=parser.nextToken();
	}

	if (arg2=="EXP")
	{
		QRegExp exp(arg3);
		if (exp.isValid())
		{
			arg2=arg3.toUpper();
			item.setIsRegExp(true);
		}
		else
		{
			reply(s, "QRegExp is not valid");
			return true;
		}

	}
	else if (item.matcherType()==AListItem::JID)
	{
		parser.back(2);
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
			Nick *n=conf->nicks()->byName(args);
			if (n && !n->jidStr().isEmpty())
				arg2=n->jidStr().section('/', 0, 0);
			else
			{
				reply(s, "JID or nick is not valid: "+args);
				return true;
			}
		}
	}

	arg2=arg2.toLower();
	item.setValue(arg2);

	if (howLong)
	{
		QDateTime t=QDateTime::currentDateTime().addSecs(howLong*60);
		item.setExpire(t);
	}

	int existsIdx=alist->indexOf(item);
	if (existsIdx>=0)
	{
		alist->removeAt(existsIdx);
	}
	
	
	if (!arg3.isEmpty())
		item.setReason(arg3);
	
	alist->append(item);
	reply(s, "Updated");
	recheckJIDs(conf);

	return true;
}

// Stanza can be null here
AListItem* MucPlugin::aFind(AList* list, Nick* nick, gloox::Stanza* s)
{
	int cnt=list->count();
	QString line;
	QString lJid=nick->jidStr().toLower().section('/', 0, 0);
	QString lNick=nick->nick().toLower();
	QString lBody;
	if (s)
		lBody=QString::fromStdString(s->body()).toLower();

	for (int i=0; i<cnt; i++)
	{
		AListItem* item=list->at(i);
	
		QString testValue;
		switch (item->matcherType())
		{
			case AListItem::UNKNOWN: break;
			case AListItem::NICK: testValue=lNick; break;
			case AListItem::JID: testValue=lJid; break;
			case AListItem::BODY: testValue=lBody; break;
		}
		if (testValue.isEmpty())
			continue;
		
		if (item->isRegExp())
		{
			QRegExp exp(item->value());
			exp.setMinimal(FALSE);
			exp.setCaseSensitivity(Qt::CaseInsensitive);
			if (exp.exactMatch(testValue))
				return item;
		}
		else
		{
			if (testValue==item->value())
				return item;
		}

	}
	return 0L;
}

void MucPlugin::checkMember(gloox::Stanza* s, Conference*c, Nick* n)
{
	if (!n)
		return;

	QString aff=n->affiliation().toUpper();
	AListItem* item;
	if ((aff!="OWNER") && !aff.startsWith("ADMIN") && aff!="MEMBER")
	{
		if (item=aFind(c->aban(), n, s))
		{
			if (s && warnImOwner(s))
				return;
			QString reason=item->reason();
			if (reason.isEmpty())
				reason=DataStorage::instance()->getString("str/ban_reason");
			setAffiliation(c, n->jidStr(), "outcast",reason);
			return;
		}

		if (item=aFind(c->akick(), n, s))
		{
			QString reason=item->reason();
			if (reason.isEmpty())
				reason=DataStorage::instance()->getString("str/kick_reason");
			setRole(c, n, "none", reason);
			return;
		}
		if (item=aFind(c->avisitor(), n, s))
		{
			QString reason=item->reason();
			if (reason.isEmpty())
				reason=DataStorage::instance()->getString("str/visitor_reason");
			setRole(c, n, "visitor", reason);
			return;
		}
	}
	if (item=aFind(c->amoderator(), n, s))
	{
		QString reason=item->reason();
		if (reason.isEmpty())
			reason=DataStorage::instance()->getString("str/moderator_reason");
		setRole(c, n, "moderator", reason);
		return;
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
int MucPlugin::getStorage(gloox::Stanza *s)
{
	Conference* conf=getConf(s);
	if (!conf)
		return 0;
	return conf->id();
}

QString MucPlugin::getJID(gloox::Stanza*s, const QString& n)
{
	qDebug() << "[MUC] getJID() " << n;
	Nick* nick=getNick(s, n);
	if (!nick)
		return QString::null;
		Conference *conf=nick->conference();
		if (!conf)
		return QString::null;
		return QString("%1/%2").arg(conf->name()).arg(nick->nick());
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
