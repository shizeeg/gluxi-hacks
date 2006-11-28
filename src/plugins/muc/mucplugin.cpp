#include "mucplugin.h"
#include "conference.h"
#include "nicklist.h"
#include "alist.h"

#include "base/common.h"
#include "base/gluxibot.h"
#include "base/datastorage.h"

#include <QtDebug>
#include <QRegExp>

#include <iostream>

#include <gloox/stanza.h>
#include <assert.h>

MucPlugin::MucPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "WHEREAMI" << "NICK" << "IDLE" << "JOIN" << "LEAVE" << "KICK" << "VISITOR" << "PARTICIPANT" << "MODERATOR";
	commands << "AKICK" << "AVISITOR" << "AMODERATOR" << "AFIND" << "SEEN";
	pluginId=1;
}


MucPlugin::~MucPlugin()
{}

void MucPlugin::onConnect()
{
	QStringList list=Conference::autoJoinList();
	QStringListIterator it(list);
	while (it.hasNext())
		join(it.next());
}

bool MucPlugin::canHandlePresence(gloox::Stanza* s)
{
	QString from=QString::fromStdString(s->from().bare());
	QString fromFull=QString::fromStdString(s->from().full());
	qDebug() << "!!!!!!" << from;
	return (conferences.byName(from) || (confInProgress.indexOf(fromFull.toUpper())>=0));
}

bool MucPlugin::isMyMessage(gloox::Stanza*s)
{
	QString jid=QString::fromStdString(s->from().bare());
	Conference* conf=conferences.byName(jid);
	if (!conf) return false;
	QString res=QString::fromStdString(s->from().resource());
	if (res.isEmpty() || conf->nick()==res)
		return true;
	return false;
}

bool MucPlugin::canHandleMessage(gloox::Stanza* s)
{
	// 	std::cout << s->xml() << std::endl;
	if (isOfflineMessage(s))
		return false;
	if (allMessages())
		return true;
	Conference *conf=getConf(s);
	if (!conf)
	{
		// Join/leave commands from owner
		QString body=getBody(s);
		QString cmd=body.section(' ',0,0).toUpper();
		if (cmd=="JOIN" || cmd=="LEAVE" || cmd=="WHEREAMI")
			return true;
		return false;
	}
	Nick *n=getNick(s);
	if (!n) return false;
	if (n->nick()==conf->nick()) return false;
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
		qDebug() << "[MUC] Got type='error' in onPresence. Looks like we can't join conference";
		confInProgress.removeAll(confFull.toUpper());
		return;
	}

	Conference* conf=getConf(s);

	//FIXME: Detect, what we shoud do if conf don't exists
	if (!conf)
	{
		conf=new Conference(confFull.section('/',0,0),confFull.section('/',1,1));
		qDebug() << conf->name();
		conferences.append(conf);
		qDebug() << confInProgress;
		confInProgress.removeAll(confFull.toUpper());
		// 		return;
	}


	Nick *n=getNick(s);
	if (!n)
	{
		if (role=="none" || type=="unavailable")
			return;
		n=new Nick(conf, nick,getItem(s,"jid"));
		conf->nicks()->append(n);
	}
	if (role=="none" || type=="unavailable")
	{
		if (n->nick()==conf->nick())
		{
			qDebug() <<"!!! I'm kicked/banned";
			conferences.remove(conf);
			return;
		}
		else
		{
			qDebug() << "!!!!!! Removing nick";
			conf->nicks()->remove(n);
		}
	}
	else
	{
		n->setAffiliation(getItem(s,"affiliation"));
		n->setJid(getItem(s,"jid"));
		n->setRole(role);
		n->setShow(getPresence(s->show()));
		n->setStatus(QString::fromStdString(s->status()));
		n->updateLastActivity();
		n->commit();

		QString confJid=QString::fromStdString(s->from().full());
		if (!confJid.isEmpty())
		{
			if (n->affiliation()=="admin" || n->affiliation()=="owner")
			{
				if (bot()->tmpOwners()->indexOf(confJid)<0)
					bot()->tmpOwners()->append(confJid);
			}
			else
			{
				bot()->tmpOwners()->removeAll(confJid);
			}
		}
		conf->removeExpired();
		checkJID(conf, n);
	}
}

bool MucPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();


	QString nickName=QString::fromStdString(s->from().resource());

	QString arg=body.section(' ',1);

	Conference* conf=getConf(s);

	// conf can be nil here!!!

	if (cmd=="JOIN")
	{
		if (!isFromOwner(s))
			return true;
		if (arg.indexOf('@')<0 || arg.indexOf('/')>=0)
		{
			reply(s,"Conference should be like \"room@server\"");
			return true;
		}
		reply(s,"Ok");
		join(arg);
		return true;
	}
	if (cmd=="LEAVE")
	{
		if (!isFromOwner(s, false))
		{
			if (isFromConfOwner(s))
			{
				if (!arg.isEmpty())
				{
					reply(s, "You can only use \"leave\" for default (your) room");
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
			reply(s,"Conference should be like \"room@server\"");
			return true;
		}
		reply(s,"Ok");
		leave(arg);
		return true;
	}

	if (cmd=="WHEREAMI")
	{
		QStringList confList;
		for (int i=0; i<conferences.count(); i++)
			confList << (conferences[i]->name().section('@',0,0)+"@");
		reply(s,QString("Currently I'm spending time at %1").arg(confList.join(", ")));
		return true;
	}

	if (!conf)
	{
		return true;
	}

	Nick *nick=getNick(s);
	if (!nick)
	{
		reply(s,QString("Looks like %1 gone...").arg(nickName));
		return true;
	}
	nick->updateLastActivity();
	nick->commit();

	if (body.isEmpty())
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
		reply(s,QString("I can see followed guys here: %1").arg(nickList.join(", ")));
		return true;
	}

	if (cmd=="NICK")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n) return true;
		reply(s, QString("Nick \"%1\": Affiliation: %2; Role: %3; Joined: %4; Idle: %5; Status: %6 (%7)")
		      .arg(n->nick())
		      .arg(n->affiliation())
		      .arg(n->role())
		      .arg(n->joined().toString(Qt::LocaleDate))
		      .arg(secsToString(n->lastActivity().secsTo(QDateTime::currentDateTime())))
		      .arg(n->show())
		      .arg(n->status())
		     );
		return true;
	}

	if (cmd=="IDLE")
	{
		Nick *n=getNickVerbose(s, arg);
		if (!n) return true;
		reply(s, QString("Idle for \"%1\" is %2").arg(n->nick()).arg(
			secsToString(n->lastActivity().secsTo(QDateTime::currentDateTime()))));
		return true;
	}

	if (cmd=="SEEN")
	{
		reply(s,conf->seen(arg));
		return true;
	}

	if (cmd=="KICK" || cmd=="VISITOR" || cmd=="PARTICIPANT" || cmd=="MODERATOR" )
	{
		if (!isFromConfModerator(s))
			return true;
		QString target;
		QString reason;
		int ps=arg.indexOf('|');
		if (ps>=0)
		{
			target=arg.section('|',0,0);
			reason=arg.section('|',1);
		}
		else
		{
			target=arg.section(' ',0,0);
			reason=arg.section(' ',1);
		}

		Nick *n=getNickVerbose(s, target);
		if (!n) return true;
		if (cmd=="KICK")
			setRole(s,n,"none",reason);
		else
			if (cmd=="VISITOR")
				setRole(s,n,"visitor",reason);
		if (cmd=="PARTICIPANT")
			setRole(s,n,"participant",reason);
		if (cmd=="MODERATOR")
			setRole(s,n,"moderator",reason);
		return true;
	}

	if (cmd=="AKICK" || cmd=="AVISITOR" || cmd=="AMODERATOR" || cmd=="AEDIT" || cmd=="AFIND")
	{
		if (!isFromConfAdmin(s))
			return true;
		return autoLists(s);
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
	if (!conf) return 0;

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
	QString jid=QString::fromStdString(s->from().bare());
	if (bot()->owners()->indexOf(jid)>=0)
		return true;
	QString nm=QString::fromStdString(s->from().resource());
	Nick *nick=getNick(s,nm);
	if (!nick) return false;

	if (nick->role()!="moderator")
	{
		reply(s,"Only conference moderator can do this.");
		return false;
	}
	return true;
}

bool MucPlugin::isFromConfAdmin(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().bare());
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
}

bool MucPlugin::isFromConfOwner(gloox::Stanza* s)
{
	QString jid=QString::fromStdString(s->from().bare());
	QString nm=QString::fromStdString(s->from().resource());
	Nick *nick=getNick(s,nm);
	if (!nick) return false;

	if (nick->affiliation()!="owner")
	{
		reply(s,"Only conference owner can do this.");
		return false;
	}
	return true;
}


void MucPlugin::setRole(gloox::Stanza* s, Nick* n, const QString& role, const QString& reason)
{
	setRole(getConf(s), n, role, reason);
}

void MucPlugin::setRole(Conference* conf, Nick* n, const QString& role, const QString& reason)
{
	if (!conf) return;

	gloox::Tag *tag=new gloox::Tag("item");
	tag->addAttribute("nick",QString(n->nick()).toStdString());
	tag->addAttribute("role",role.toStdString());

	gloox::Tag *reas=new gloox::Tag("reason",reason.toStdString());
	tag->addChild(reas);

	gloox::Stanza *st=gloox::Stanza::createIqStanza(
	                      conf->name().toStdString(),
	                      "someID",
	                      gloox::StanzaIqSet,
	                      "http://jabber.org/protocol/muc#admin",
	                      tag
	                  );
	std::cout << st->xml() << std::endl << std::endl;
	bot()->client()->send(st);
}

Nick* MucPlugin::getNickVerbose(gloox::Stanza* s, const QString& nn)
{
	Nick *nick=getNick(s,nn);
	if (!nick)
	{
		reply(s,QString("Nick \"%1\" not found").arg(nn));
	}
	return nick;
}

void MucPlugin::join(const QString& name)
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
		cname=confName.section('/',0,0);
		cnick=confName.section('/',1,1);
	}

	if (confInProgress.indexOf(confName.toUpper())>=0)
	{
		qWarning() << "Joining already in progress";
		return;
	}

	confInProgress.append(confName.toUpper());

	// Don't create "Conference" object, because it's possible that we can't join it
	// 	Conference *conf=new Conference(cname,cnick);
	// 	conferences.append(conf);
	gloox::Stanza *st=gloox::Stanza::createPresenceStanza(gloox::JID(confName.toStdString()));

	// Be MUC compatible according to XEP-0045
	gloox::Tag *tg=new gloox::Tag("x");
	tg->addAttribute("xmlns","http://jabber.org/protocol/muc");
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
		cname=confName.section('/',0,0);
		cnick=confName.section('/',1,1);
	}

	Conference*conf=conferences.byName(cname);
	if (conf)
		conf->setAutoJoin(FALSE);

	/*	Conference *conf=new Conference(cname,cnick);
		conferences.append(conf); */

	gloox::Stanza *st=gloox::Stanza::createPresenceStanza(gloox::JID(confName.toStdString()));
	st->addAttribute("type","unavailable");

	std::cout << st->xml() << std::endl << std::endl;
	bot()->client()->send(st);
}

QString MucPlugin::getItem(gloox::Stanza* s, const QString& name)
{
	std::string nname=name.toStdString();
	std::string res;
	if (s->hasChild("x","xmlns","http://jabber.org/protocol/muc#user"))
	{
		gloox::Tag *tg1=s->findChild("x","xmlns","http://jabber.org/protocol/muc#user");
		assert(tg1);
		if (tg1->hasChild("item",nname))
		{
			gloox::Tag *tg2=tg1->findChild("item",nname);
			assert(tg2);
			res=tg2->findAttribute(nname);
		}
	}
	return QString::fromStdString(res);
}

QString MucPlugin::getPresence(const gloox::Presence& pr)
{
	switch (pr)
	{
	case gloox::PresenceAvailable : return "Available";
	case gloox::PresenceAway: return "Away";
	case gloox::PresenceChat: return "Chat";
	case gloox::PresenceDnd: return "Dnd";
	case gloox::PresenceUnavailable: return "Unavailiable";
	case gloox::PresenceXa: return "Xa";
	case gloox::PresenceUnknown: return "Unknown";
	}
	return "Unknown";
}

bool MucPlugin::canHandleIq( gloox::Stanza* /* s */)
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
	if (!conf) return false;
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


bool MucPlugin::autoLists(gloox::Stanza *s)
{
	Conference* conf=getConf(s);
	QString body=getBody(s);
	qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!! body=" << body;

	int narg=0;
	QString arg=body.section(' ',narg,narg).toUpper(); narg++;
	qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!! arg=" << arg;
	QString nickName=QString::fromStdString(s->from().resource());

	AList* alist=0;
	QString arg2=body.section(' ',narg,narg).toUpper();
	if (arg=="AFIND")
	{
		QString answer;

		if (aFind(conf->akick(),arg2))
			answer+=QString("\"%1\" is in akick list\n").arg(arg2.toLower());
		if (aFind(conf->avisitor(),arg2))
			answer+=QString("\"%1\" is in avisitor list\n").arg(arg2.toLower());
		if (aFind(conf->amoderator(),arg2))
			answer+=QString("\"%1\" is in amoderator list\n").arg(arg2.toLower());
		if (answer.endsWith("\n"))
			answer.remove(answer.length()-1,1);
		if (answer.isEmpty())
			answer=QString("\"%1\" is not found in a-lists").arg(arg2.toLower());
		reply(s,answer);
		return true;
	}

	if (arg=="AKICK")
		alist=conf->akick();
	if (arg=="AVISITOR")
		alist=conf->avisitor();
	if (arg=="AMODERATOR")
		alist=conf->amoderator();
	if (!alist)
	{
		reply(s,QString("Try \"!muc %1 help").arg(arg.toLower()));
		return TRUE;
	}
	arg=arg.toLower();
	QString args=body.section(' ',narg);
	arg2=body.section(' ',narg,narg).toUpper(); narg++;
	QString arg3=body.section(' ',narg);
	if (arg2=="HELP" || arg2=="")
	{
		reply(s,QString("\"%1\" commands: help, show, count, clear, del <item>, exp <regexp>, <jid>").arg(arg));
		return TRUE;
	}
	if (arg2=="SHOW")
	{
		alist->removeExpired();
		if (!alist->count())
			reply(s, QString("\"%1\" list is empty").arg(arg));
		else
		{
			reply(s,QString("\"%1\" list:\n%2").arg(arg).arg(alist->toString()));
		}
		return TRUE;
	}
	if (arg2=="COUNT")
	{
		reply(s,QString("Currntly list \"%1\" contains %2 items").arg(arg).arg(alist->count()));
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
			n=alist->removeAll(arg3.toUpper());
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
		else
			if (arg2u.endsWith("H"))
				k=60;
			else
				if (arg2u.endsWith("M"))
					k=1;
		if (k!=0)
		{
			arg2.remove(arg2.length()-1,1);
			qDebug() << arg2;
		}
		arg2.remove(0,1);
		if (k==0)
			k=1;
		bool ok;
		int dt=arg2.toInt(&ok);
		if (!ok)
		{
			reply(s,"time is invalid. Should be NUM[d|h|m]");
			return true;
		}
		howLong=dt*k;
		arg2=body.section(' ',narg,narg).toUpper(); narg++;
		arg3=body.section(' ',narg);
	}

	bool isNick=false;
	bool isJid=false;
	if (arg2=="NICK")
	{
		arg2=body.section(' ',narg,narg).toUpper(); narg++;
		arg3=body.section(' ',narg);
		qDebug() << arg2;
		qDebug() << arg3;
		isNick=true;
	}
	else
		if (arg2=="JID")
		{
			arg2=body.section(' ',narg,narg).toUpper(); narg++;
			arg3=body.section(' ',narg);
			isJid=true;
		}

	if (arg2=="EXP")
	{
		QRegExp exp(arg3);
		if (exp.isValid())
			arg2=QString("exp "+arg3).toUpper();
		else
		{
			reply(s,"QRegExp is not valid");
			return true;
		}
		qDebug() << arg2;
	}
	else
		if (!isNick)
		{
			args=QString(arg2+" "+arg3).trimmed();
			QRegExp exp("[^@ ]*@[^ ]*");
			exp.setMinimal(FALSE);
			int ps=exp.indexIn(args);
			// 			qDebug() << arg2 << " | "<< "ps=" << ps << "  " << exp.matchedLength() << "   " << arg2.length() << " !! " << exp.isValid();
			if (ps==0 && exp.matchedLength()==args.length())
			{
				// All ok
			}
			else
			{
				qDebug() << args;
				Nick *n=conf->nicks()->byName(args);
				if (n && !n->jid().isEmpty())
					arg2=n->jid().section('/',0,0);
				else
				{
					reply(s, "JID or nick is not valid");
					return true;
				}
			}
		}

	qDebug() << "[]]]]]] " << arg2;
	arg2=arg2.toUpper();
	if (isNick)
		arg2="NICK "+arg2;

	if (alist->indexOf(arg2)>=0)
	{
		// 		reply(s, "Entry already exists. ");
		alist->removeAll(arg2);
	}

	if (howLong)
	{
		QDateTime t=QDateTime::currentDateTime().addSecs(howLong*60);
		alist->append(arg2,t);
	}
	else
		alist->append(arg2);
	reply(s, "Updated");
	recheckJIDs(conf);

	return true;
}

bool MucPlugin::aFind(AList* list, const QString& jid, bool nickOnly)
{
	int cnt=list->count();
	QString line;
	QString uJID=jid.toUpper();
	for (int i=0; i<cnt; i++)
	{
		line=list->at(i);
		if (line.startsWith("NICK "))
		{
			line=line.section(' ',1);
		}
		else
		{
			if (nickOnly)
				continue;
		}

		if (line.startsWith("EXP "))
		{
			QRegExp exp(line.section(' ',1));
			exp.setMinimal(FALSE);
			exp.setCaseSensitivity(Qt::CaseInsensitive);
			int ps=exp.indexIn(uJID);
			if (ps==0 && exp.matchedLength()==jid.count())
				return TRUE;
		}
		else
		{
			if (line==uJID)
				return TRUE;
		}
	}
	return FALSE;
}

void MucPlugin::checkNick(Conference*c , Nick* n, const QString& jid, bool nickOnly)
{
	if (!jid.isEmpty())
	{
		if (aFind(c->akick(), jid,nickOnly))
		{
			setRole(c, n, "none", "You are not welcomed here");
			return;
		}
		if (aFind(c->avisitor(), jid,nickOnly))
		{
			setRole(c, n, "visitor", "You shoud be a visitor");
			return;
		}
		if (aFind(c->amoderator(), jid,nickOnly))
		{
			setRole(c, n, "moderator", "You shoud be a moderator");
			return;
		}
	}
}

void MucPlugin::checkJID(Conference* c, Nick *n)
{
	QString jid=n->jid().section('/',0,0);
	qDebug() << "!!! Checking jid " << jid;
	checkNick(c,n, jid);
	checkNick(c,n,n->nick(),true);
}

void MucPlugin::recheckJIDs(Conference *c)
{
	c->removeExpired();
	int cnt=c->nicks()->count();
	for (int i=0; i<cnt; i++)
	{
		Nick *n=c->nicks()->at(i);
		assert(n);
		checkJID(c,n);
	}
}

// Storage provider
int MucPlugin::getStorage( gloox::Stanza *s)
{
	Conference* conf=getConf(s);
	if (!conf)
		return 0;
	return conf->id();
}

QString MucPlugin::getJID(gloox::Stanza*s, const QString& n)
{
	qDebug() << "[MUC] getJID() " << n;
	Nick* nick=getNick(s,n);
	if (!nick)
	        return QString::null;
	Conference *conf=nick->conference();
	if (!conf)
		return QString::null;
	
	return QString("%1/%2").arg(conf->name()).arg(nick->nick());
}

QString MucPlugin::JIDtoNick(const QString& jid)
{
	QString c=jid.section('/',0,0);
	QString n=jid.section('/',1);
	Conference *conf=conferences.byName(c);
	if (conf)
		return n;
	return QString::null;
}

void MucPlugin::sendMessage(Conference *conf, const QString& msg)
{
	if (!conf) return;
	QString jid=conf->name();
	
	gloox::Stanza *st=gloox::Stanza::createMessageStanza(
		gloox::JID(jid.toStdString()), msg.toStdString());
	st->addAttribute("type","groupchat");
	bot()->client()->send(st);
}

void MucPlugin::onQuit(const QString& reason)
{
	int cnt=conferences.count();
	for (int i=0; i<cnt; i++)
	{
		Conference *conf=conferences[i];
		sendMessage(conf,QString("Shutting down (%1)").arg(reason));		
	}
}

