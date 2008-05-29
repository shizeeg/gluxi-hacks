#include "configplugin.h"
#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/config/abstractconfigurator.h"

#include <gloox/stanza.h>

#include <QtDebug>
#include <QTime>

ConfigPlugin::ConfigPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	bot()->client()->registerIqHandler("http://jabber.org/protocol/disco#info");
	bot()->client()->registerIqHandler("http://jabber.org/protocol/commands");
}

ConfigPlugin::~ConfigPlugin()
{
}

bool ConfigPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	qDebug() << "Got CMD: " << cmd << "; length=" << cmd.length();
	
	return false;
}

bool ConfigPlugin::canHandleIq(gloox::Stanza* s)
{
	return true;
}

bool ConfigPlugin::onIq(gloox::Stanza* s)
{
	AbstractConfigurator* config=bot()->getConfigurator(s);
	if (!config)
		return false;
	
	if (s->subtype() == gloox::StanzaIqGet)
	{
		gloox::Tag* queryTag=s->findChild("query");
		if (queryTag==0)
			return false;
		
		if (s->xmlns() == "http://jabber.org/protocol/disco#info")
		{
			//Notify about commands support
			gloox::Stanza* out=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult,"http://jabber.org/protocol/disco#info");
			queryTag=out->findChild("query");
			if (queryTag==0)
				return false;
			queryTag->addChild(new gloox::Tag("feature","var","http://jabber.org/protocol/commands",false));
			out->finalize();
			bot()->client()->send(out);
			return true;
		}
		if (s->xmlns() == "http://jabber.org/protocol/disco#items" && s->findChild("query","node","http://jabber.org/protocol/commands"))
		{
			qDebug() << "Report command list";
			gloox::Stanza* out=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult,"http://jabber.org/protocol/disco#items");
			queryTag=out->findChild("query");
			if (queryTag==0)
				return false;
			
			queryTag->addChild(createCommandTag("config","Configuration",config->targetJid()));
			
			out->finalize();
			bot()->client()->send(out);
		}
	}
	if (s->subtype()==gloox::StanzaIqSet || s->subtype() == gloox::StanzaIqGet)
	{
		gloox::Tag* incCmdTag=s->findChild("command","node","http://jabber.org/protocol/rc#config");
		if (incCmdTag)
		{
			QString action=QString::fromStdString(incCmdTag->findAttribute("action")).toLower();
			bool isCancel=(action=="cancel");
			
			gloox::Stanza* st=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult);
			gloox::Tag *cmdTag=new gloox::Tag("command");
			cmdTag->addAttribute("xmlns","http://jabber.org/protocol/commands");
			cmdTag->addAttribute("node","http://jabber.org/protocol/rc#config");
			cmdTag->addAttribute("sessionid",s->id());
			
			if (isCancel)
			{
				cmdTag->addAttribute("status","canceled");
			}
			else
			{
				cmdTag->addAttribute("status","executing");
				gloox::Tag *xTag=new gloox::Tag("x");
				xTag->addAttribute("xmlns","jabber:x:data");
				xTag->addAttribute("type","form");
				xTag->addChild(new gloox::Tag("title","Form title"));
				xTag->addChild(new gloox::Tag("instructions","Manual how to fill"));
				QList<ConfigField> fields=config->loadFields();
				for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
				{
					xTag->addChild(createFieldTag(*it));
				}
				cmdTag->addChild(xTag);
			}
			st->addChild(cmdTag);
			st->finalize();
			qDebug() << QString::fromStdString(st->xml());
			bot()->client()->send(st);
			return true;
		}
	}
}

gloox::Tag* ConfigPlugin::createCommandTag(const QString& nodePart, const QString& name, const QString& jid)
{
	gloox::Tag* tag=new gloox::Tag("item");
	tag->addAttribute("node",QString("http://jabber.org/protocol/rc#%1").arg(nodePart).toStdString());
	tag->addAttribute("name",name.toStdString());
	tag->addAttribute("jid",jid.toStdString());
	return tag;
}

gloox::Tag* ConfigPlugin::createFieldTag(const ConfigField& field)
{
	gloox::Tag *fieldTag=new gloox::Tag("field");
	fieldTag->addAttribute("type",fieldTypeToString(field.type()).toStdString());
	fieldTag->addAttribute("var",field.name().toStdString());
	fieldTag->addAttribute("label",field.description().toStdString());
	fieldTag->addChild(new gloox::Tag("value",field.value().toStdString()));
	fieldTag->addChild(new gloox::Tag("required"));
	return fieldTag;
}

QString ConfigPlugin::fieldTypeToString(ConfigField::FieldType fieldType)
{
	switch (fieldType)
	{
	case ConfigField::FIELDTYPE_UNKNOWN:
	case ConfigField::FIELDTYPE_TEXT:
		return "text-single";
	case ConfigField::FIELDTYPE_CHECKBOX:
		return "checkbox";
	}
	return QString();
}
