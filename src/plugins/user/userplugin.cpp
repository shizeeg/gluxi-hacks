#include "userplugin.h"
#include "image2ascii.h"
#include "base/common.h"
#include "base/glooxwrapper.h"
#include "base/gluxibot.h"
#include "base/asyncrequestlist.h"
#include "base/messageparser.h"
#include "base/gluxi_version.h"

#include "base/gloox/myvcardmanager.h"
#include <string>

#include <QFile>
#include <QtDebug>
#include <QTime>
#include <QImage>

//#include <gloox/client.h>

#include <assert.h>

UserPlugin::UserPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "VERSION" << "PING" << "DISCO" << "VCARD" << "PHOTO";

	bot()->registerIqHandler("jabber:iq:version");
	bot()->registerIqHandler("http://jabber.org/protocol/disco#items");
}

UserPlugin::~UserPlugin()
{
}

bool UserPlugin::canHandleIq(gloox::Stanza* s)
{
	QString xmlns=QString::fromStdString(s->xmlns());
	if (xmlns=="jabber:iq:version")
		return true;
	return false;
}

bool UserPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	QString arg=parser.nextToken();

	if (cmd=="VERSION" || cmd=="PING")
	{
		std::string id=bot()->client()->getID();
		QString jid=bot()->getJID(s, arg);
		if (jid.isEmpty())
			jid=arg;
		if (jid.isEmpty())
			jid=QString::fromStdString(s->from().full());

		gloox::Stanza *st=gloox::Stanza::createIqStanza(
				gloox::JID(jid.toStdString()), id, gloox::StanzaIqGet,
				"jabber:iq:version");

		qDebug() << QString::fromStdString(st->xml());

		gloox::Stanza *sf=new gloox::Stanza(s);
		sf->addAttribute("id", id);

		AsyncRequest *req=new AsyncRequest(-1, this, sf, 3600);
		req->setName(cmd);
		bot()->asyncRequests()->append(req);
		bot()->client()->send(st);
		return true;
	}
	if (cmd=="DISCO")
	{
		std::string id=bot()->client()->getID();
		QString jid=bot()->getJID(s, arg);
		if (jid.isEmpty())
			jid=arg;
		if (jid.isEmpty())
			jid=QString::fromStdString(s->from().full());
		gloox::Stanza *st=gloox::Stanza::createIqStanza(
				gloox::JID(jid.toStdString()), id, gloox::StanzaIqGet,
				"http://jabber.org/protocol/disco#items");
		gloox::Stanza *sf=new gloox::Stanza(s);
		sf->addAttribute("id", id);
		AsyncRequest *req=new AsyncRequest(-1, this, sf, 3600);
		req->setName(jid);
		bot()->asyncRequests()->append(req);
		bot()->client()->send(st);
		return true;
	}
	if (cmd=="VCARD" || cmd=="PHOTO")
	{
		QString jid=bot()->getJID(s, arg);
		if (jid.isEmpty())
			jid=arg;
		if (jid.isEmpty())
			jid=QString::fromStdString(s->from().bare());
		QString vcardId=bot()->client()->fetchVCard(jid);
		gloox::Stanza *sf=new gloox::Stanza(s);
		qDebug() << "VCard request: " << jid;
		sf->addAttribute("id", QString("vcard_%1").arg(vcardId).toStdString());
		AsyncRequest *req=new AsyncRequest(-1, this, sf, 3600);
		req->setName(cmd);
		bot()->asyncRequests()->append(req);
		return true;
	}

	return false;
}

void UserPlugin::sendVersion(gloox:: Stanza* s)
{
	gloox::Stanza *st=gloox::Stanza::createIqStanza(s->from(),
			s->findAttribute("id"), gloox::StanzaIqResult, "jabber:iq:version");
	gloox::Tag* tag=st->findChild("query");
	assert(tag);
	tag->addChild(new gloox::Tag("name","GluxiBot"));
	tag->addChild(new gloox::Tag("version",getGluxiVersion()));
	tag->addChild(new gloox::Tag("os",version().toStdString()));
	qDebug() << QString::fromStdString(st->xml());
	bot()->client()->send(st);
}

bool UserPlugin::onIq(gloox::Stanza* s)
{
	QString xmlns=QString::fromStdString(s->xmlns());
	AsyncRequest* req=bot()->asyncRequests()->byStanza(s);
	if (s->subtype()==gloox::StanzaIqGet)
	{
		if (s->subtype()!=gloox::StanzaIqResult && xmlns=="jabber:iq:version")
		{
			//We should send our version
			sendVersion(s);
			return true;
		}
		if (xmlns=="http://jabber.org/protocol/disco#items")
		{
			//Disco items request. Report error;
			gloox::Stanza *st=gloox::Stanza::createIqStanza(s->from(),
					s->findAttribute("id"), gloox::StanzaIqError,
					xmlns.toStdString());
			bot()->client()->send(st);
		}
		if (req)
			bot()->asyncRequests()->removeAll(req);
		return true;
	}

	if (!req)
		return false;
	if (req->plugin()!=this)
		return false;

	gloox::Tag* query=s->findChild("query", "xmlns", xmlns.toStdString());
	if (!query)
	{
		reply(req->stanza(), "Error");
		bot()->asyncRequests()->removeAll(req);
		delete req;
		return true;
	}
	if (xmlns=="jabber:iq:version")
	{
		if (req->name()=="PING")
		{
			QString msg;
			QString src=bot()->JIDtoNick(QString::fromStdString(s->from().full()));
			double delay=(double)req->time().time().msecsTo(QTime::currentTime());
			delay/=1000.0;

			if (s->subtype()==gloox::StanzaIqResult)
				msg=QString("Pong from %1 after %2 secs.").arg(src).arg(delay);
			else
				msg=QString("Pong from %1's server after %2 secs.").arg(src).arg(delay);
			reply(req->stanza(), msg);
		}
		else if (s->subtype()==gloox::StanzaIqResult)
		{
			QString name;
			QString version;
			QString os;
			gloox::Tag* t;

			t=query->findChild("name");
			if (t)
				name=QString::fromStdString(t->cdata());
			t=query->findChild("version");
			if (t)
				version=QString::fromStdString(t->cdata());
			t=query->findChild("os");
			if (t)
				os=QString::fromStdString(t->cdata());
			QString res=name;
			if (!version.isEmpty())
				res+=QString(" %1").arg(version);
			if (!os.isEmpty())
				res+=QString(" // %1").arg(os);

			QString src=bot()->JIDtoNick(QString::fromStdString(s->from().full()));
			if (!src.isEmpty())
				src+=" uses ";
			src+=res;

			reply(req->stanza(), src);
		}
		else
		{
			//TODO: Error handling
			reply(req->stanza(), "Unable to get version");
		}
	}
	if (xmlns=="http://jabber.org/protocol/disco#items")
	{
		if (s->subtype()==gloox::StanzaIqError)
			reply(req->stanza(), "Unable to query");
		else
		{
			QList<gloox::Tag*> lst=
					QList<gloox::Tag*>::fromStdList(query->children());
			QStringList strings;
			bool safeJid=req->name().indexOf('@')<0;
			int noval=0;
			for (int i=0; i<lst.count(); i++)
			{
				QString name=
						QString::fromStdString(lst[i]->findAttribute("name"));
				QString jid=
						QString::fromStdString(lst[i]->findAttribute("jid"));
				if (name.isEmpty())
					name=jid;
				if (name.isEmpty())
					continue;
				QString cnt=getValue(name, "^.*\\(([0-9]+)\\)$");
				if (cnt.isEmpty())
				{
					cnt="0";
					noval++;
				}
				else
					name=getValue(name,"^(.*)\\([0-9]+\\)$").trimmed();
				cnt=cnt.rightJustified(8, '0', true);
				if (name!=jid && safeJid)
					name+=QString(" [%1]").arg(jid);
				strings.append(QString("%1 %2").arg(cnt).arg(name));
			}
			qDebug() << strings;
			strings.sort();
			QStringList replyList;
			bool haveValues;
			if (lst.count())
			{
				double perc=((double)noval)/((double)(lst.count()));
				haveValues=perc<0.2;
			}

			int idx=strings.count()-1;
			for (int i=0; i<strings.count(); i++)
			{
				int curIdx=haveValues ? idx : i;
				int value=strings[curIdx].section(' ',0,0).toInt();
				QString name=strings[curIdx].section(' ', 1);
				idx--;
				QString item=QString("%1) %2").arg(i+1).arg(name);
				if (haveValues)
					item+=QString(": %1").arg(value);
				replyList.append(item);
				if (replyList.count()>=100)
				{
					replyList.append("...");
					break;
				}
			}
			reply(req->stanza(), "Disco items:\n"+replyList.join("\n"));
		}
	}

	bot()->asyncRequests()->removeAll(req);
	delete req;
	return true;
}

bool UserPlugin::onVCard(const VCardWrapper& vcardWrapper)
{
	const gloox::JID jid=vcardWrapper.jid();
	gloox::VCard vcard=gloox::VCard(vcardWrapper.vcard());

	qDebug() << "Got vcard: "+vcardWrapper.id();
	QString jidStr=QString::fromStdString(jid.full());
	QString reqId=QString("vcard_%1").arg(vcardWrapper.id());
	AsyncRequest* req=bot()->asyncRequests()->byStanzaId(reqId);
	if (req==0l)
	{
		return false;
	}
	if (vcardWrapper.isEmpty())
	{
		reply(req->stanza(), "No VCard found");
		bot()->asyncRequests()->removeAll(req);
		return true;
	}

	if (req->name()=="VCARD")
	{
		QString replyStr=vcardWrapper.vcardStr();
		if (replyStr.isEmpty())
		{
			reply(req->stanza(), "Empty VCard");
		}
		else
		{
			reply(req->stanza(), QString("VCard: %1").arg(replyStr));
		}
	}
	else if (req->name()=="PHOTO")
	{
		std::string photoContentStd=vcard.photo().binval;
		QByteArray photoContent=QByteArray(photoContentStd.data(),
				photoContentStd.size());
		//QFile file("/tmp/out.png");
		//file.open(QIODevice::WriteOnly);
		//file.write(photoContentStd.data(), photoContentStd.size());
		//file.close();
		QImage image;
		if (!image.loadFromData(photoContent))
		{
			reply(req->stanza(), "Can't load image");
			bot()->asyncRequests()->removeAll(req);
			return true;
		}

		MessageParser parser(req->stanza(), getMyNick(req->stanza()));
		parser.nextToken();
		QString cmd=parser.nextToken().toUpper();
		QString jid=parser.nextToken();
		QString widthStr=parser.nextToken();
		QString white=parser.nextToken();
		QString black=parser.nextToken();

		Image2Ascii img2ascii(image);

		if (!widthStr.isEmpty())
			img2ascii.setWidth(widthStr.toInt());
		if (!white.isEmpty() && white.length()<=5)
			img2ascii.setWhite(white);
		if (!black.isEmpty() && black.length()<=5)
			img2ascii.setBlack(black);		
		
		QString ascii=img2ascii.ascii();
		if (ascii.isEmpty())
			reply(req->stanza(), "Can't convert image to ASCII");
		else
			reply(req->stanza(), QString("Photo:\n%1").arg(ascii));
	}
	bot()->asyncRequests()->removeAll(req);
	return true;
}
