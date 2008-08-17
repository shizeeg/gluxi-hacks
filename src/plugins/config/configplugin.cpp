#include "configplugin.h"
#include "base/messageparser.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/config/abstractconfigurator.h"
#include "base/disco/rootdiscohandler.h"
#include "base/disco/featureitem.h"
#include "base/disco/identityitem.h"

#include <gloox/stanza.h>
#include <list>

#include <QtDebug>
#include <QTime>

#define GLUXI_CONFIG_NODE "http://gluxi.inhex.net/node/config"

ConfigPlugin::ConfigPlugin(GluxiBot *parent) :
	BasePlugin(parent),  DiscoHandler(GLUXI_CONFIG_NODE,"", "Configuration")
{
	bot()->registerIqHandler("http://jabber.org/protocol/disco#info");
	bot()->registerIqHandler("http://jabber.org/protocol/commands");
	bot()->rootDiscoHandler()->registerDiscoHandler(this);
	addInfoItem(new FeatureItem("http://jabber.org/protocol/commands"));
	addInfoItem(new IdentityItem("automation", "command-node", "Configuration"));
	commands << "GET" << "SET";
}

ConfigPlugin::~ConfigPlugin()
{
	bot()->rootDiscoHandler()->unregisterDiscoHandler(this);
}

bool ConfigPlugin::parseMessage(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();
	if (cmd=="GET")
	{
		AbstractConfigurator* config=getConfiguratorVerbose(s);
		if (!config)
			return true;
		QString name=parser.nextToken();
		QList<ConfigField> fields=config->loadFields();
		QString res;
		int idx=0;
		for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
		{
			ConfigField field = *it;
			if (name.isEmpty())
				res+=QString("\n%1) %2").arg(++idx).arg(field.name()+": "+field.value());
			else
			{
				if (name.toLower()==field.name().toLower())
					res=field.name()+": "+field.value();
			}
		}
		if (res.isEmpty())
			reply(s, "No configuration field(s) found");
		else
		{
			reply(s, res);
		}
		return true;
	}

	if (cmd=="SET")
	{
		AbstractConfigurator* config=getConfiguratorVerbose(s);
		if (!config)
			return true;
		QString name;
		QString value;

		name=parser.nextToken().trimmed();
		value=parser.nextToken().trimmed();
		if (name.isEmpty() || value.isEmpty())
		{
			reply(s, "Incorrect name or value");
			return true;
		}
		QList<ConfigField> fields=config->loadFields();
		QList<ConfigField> modifiedList;
		for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
		{
			ConfigField field = *it;
			if (name.toLower()==field.name())
			{
				field.setValue(value);
				modifiedList.append(field);
				break;
			}
		}
		if (!modifiedList.isEmpty())
		{
			config->saveFields(modifiedList);
			reply(s, "Updated");
		}
		else
		{
			reply(s, "No field found");
		}
		return true;
	}
	return false;
}

gloox::Stanza* ConfigPlugin::handleDiscoRequest(gloox::Stanza* s, const QString& jid)
{
	gloox::Stanza* res=0;
	if (res=DiscoHandler::handleDiscoRequest(s, jid))
		return res;

	AbstractConfigurator* config=bot()->getConfigurator(s);
	if (!config)
		return 0;

	if (s->subtype()==gloox::StanzaIqSet || s->subtype() == gloox::StanzaIqGet)
	{
		gloox::Tag* incCmdTag=s->findChild("command","node", GLUXI_CONFIG_NODE);
		if (incCmdTag)
		{
			gloox::Tag* xTag=incCmdTag->findChild("x");

			QString action=QString::fromStdString(incCmdTag->findAttribute("action")).toLower();
			bool isCancel=(action=="cancel");

			gloox::Stanza* st=gloox::Stanza::createIqStanza(s->from(),s->id(),gloox::StanzaIqResult);
			gloox::Tag *cmdTag=new gloox::Tag(incCmdTag->name());
			cmdTag->addAttribute("xmlns",cmdTag->findAttribute("xmlns"));
			cmdTag->addAttribute("node",GLUXI_CONFIG_NODE);
			cmdTag->addAttribute("sessionid",s->id());

			if (isCancel)
			{
				cmdTag->addAttribute("status","canceled");
			}
			else
			{
				if (xTag && xTag->findAttribute("type")=="submit")
				{
					std::list<gloox::Tag*> childTags=xTag->children();
					QList<ConfigField> configFieldList;
					for (std::list<gloox::Tag*>::iterator it=childTags.begin(); it!=childTags.end(); ++it)
					{
						gloox::Tag* child=(*it);
						if (child->name()!="field")
							continue;
						configFieldList.append(createConfigFieldFromTag(child));
					}
					config->saveFields(configFieldList);
					cmdTag->addAttribute("status","completed");
				}
				else
				{
					cmdTag->addAttribute("status","executing");
					gloox::Tag *xTag=new gloox::Tag("x");
					xTag->addAttribute("xmlns","jabber:x:data");
					xTag->addAttribute("type","form");
					xTag->addChild(new gloox::Tag("title","Configuration"));
					xTag->addChild(new gloox::Tag("instructions","Please fill all fields"));
					QList<ConfigField> fields=config->loadFields();
					for (QList<ConfigField>::iterator it=fields.begin(); it!=fields.end(); ++it)
					{
						xTag->addChild(createFieldTag(*it));
					}
					cmdTag->addChild(xTag);
				}
			}
			st->addChild(cmdTag);
			st->finalize();
			return st;
		}
	}
	return 0;
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
		return "boolean";
	}
	return QString();
}

ConfigField ConfigPlugin::createConfigFieldFromTag(gloox::Tag* tag)
{
	QString type("");
	QString name=QString::fromStdString(tag->findAttribute("var"));
	QString value;
	gloox::Tag* valueTag=tag->findChild("value");
	if (valueTag)
		value=QString::fromStdString(valueTag->cdata());
	return ConfigField(name,value);
}

AbstractConfigurator* ConfigPlugin::getConfiguratorVerbose(gloox::Stanza* s)
{
	AbstractConfigurator* config=bot()->getConfigurator(s);
	if (!config)
	{
		reply(s,"Unable to find configurator");
		return 0;
	}
	return config;
}
