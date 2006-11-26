#include "userplugin.h"
#include "base/common.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"


#include <QtDebug>
#include <QTime>

#include <assert.h>

UserPlugin::UserPlugin(GluxiBot *parent)
		: BasePlugin(parent)
{
	commands << "VERSION";
	bot()->client()->registerIqHandler(bot(),"jabber:iq:version");
}


UserPlugin::~UserPlugin()
{}

bool UserPlugin::canHandleIq(gloox::Stanza* s)
{
	QString xmlns=QString::fromStdString(s->xmlns());
	if (xmlns=="jabber:iq:version")
		return true;
	return false;
}

bool UserPlugin::parseMessage(gloox::Stanza* s)
{
	QString body=getBody(s);
	QString cmd=body.section(' ',0,0).toUpper();
	QString arg=body.section(' ',1).toUpper();

	if (cmd=="VERSION")
	{
		std::string id=bot()->client()->getID();
		QString jid=bot()->getJID(s,arg);
		qDebug() << "!!!!!!!!! JID=" << jid;
		if (jid.isEmpty())
			jid=arg;
		if (jid.isEmpty())
			jid=QString::fromStdString(s->from().full());
	
		gloox::Stanza *st=gloox::Stanza::createIqStanza(
			gloox::JID(jid.toStdString()),
//			bot()->client()->getID(),
			id,
			gloox::StanzaIqGet,
			"jabber:iq:version");

		qDebug() << QString::fromStdString(st->xml());

		AsyncRequest *req=new AsyncRequest(this, st->clone());
		req->setSource(s->clone());
		bot()->asyncRequests()->append(req);
		bot()->client()->send(st);
		return true;
	}
	return false;
}

void UserPlugin::sendVersion(gloox:: Stanza* s)
{
	gloox::Stanza *st=gloox::Stanza::createIqStanza(
		s->from(),
		s->findAttribute("id"),
		gloox::StanzaIqResult,
		"jabber:iq:version");
	gloox::Tag* tag=st->findChild("query");
	assert(tag);
	tag->addChild(new gloox::Tag("name","GluxiBot"));
	tag->addChild(new gloox::Tag("version","SVN"));
	tag->addChild(new gloox::Tag("os",version().toStdString()));
	qDebug() << QString::fromStdString(st->xml());
	bot()->client()->send(st);
}

bool UserPlugin::onIq(gloox::Stanza* s)
{
	QString xmlns=QString::fromStdString(s->xmlns());
	if (xmlns=="jabber:iq:version" &&
		s->subtype()==gloox::StanzaIqGet)
	{
		//We should send our version
		sendVersion(s);
		return true;
	}

	AsyncRequest* req=bot()->asyncRequests()->byStanza(s);
	if (!req)
		return false;
	if (req->plugin()!=this)
		return false;

	gloox::Tag* query=s->findChild("query","xmlns",xmlns.toStdString());
	if (!query)
	{
		reply(req->source(),"Error");
		bot()->asyncRequests()->removeAll(req);
		delete req;
		return true;
	}
	if (xmlns=="jabber:iq:version")
	{
		if (s->subtype()==gloox::StanzaIqResult)
		{
			QString name;
			QString version;
			QString os;
			gloox::Tag* t;

			t=query->findChild("name");
			if (t) name=QString::fromStdString(t->cdata());
			t=query->findChild("version");
			if (t) version=QString::fromStdString(t->cdata());
			t=query->findChild("os");
			if (t) os=QString::fromStdString(t->cdata());
			QString res=name;
			if (!version.isEmpty()) res+=QString(" %1").arg(version);
			if (!os.isEmpty()) res+=QString(" // %1").arg(os);
			
			QString src=bot()->JIDtoNick(QString::fromStdString(
					s->from().full()));
			if (!src.isEmpty())
				src+=" use ";
			src+=res;

			reply(req->source(),src);
		}
		else
		{
			//TODO: Error handling
			reply(req->source(),"Unable to get version");
		}
	}
	bot()->asyncRequests()->removeAll(req);
	delete req;
	return true;
}

