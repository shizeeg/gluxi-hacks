#include "aliasplugin.h"
#include "base/gluxibot.h"
#include "base/glooxwrapper.h"
#include "base/rolelist.h"
#include "base/messageparser.h"
#include "base/common.h"

#include <QList>
#include <QtDebug>
#include <QTextCodec>

#include <assert.h>

AliasPlugin::AliasPlugin(GluxiBot *parent) :
	BasePlugin(parent)
{
	commands << "SHOW" << "CLEAR" << "COUNT" << "ADD" << "DEL";
}

AliasPlugin::~AliasPlugin()
{
}

bool AliasPlugin::canHandleMessage(gloox::Stanza* s, const QStringList& flags)
{
	if (isOfflineMessage(s))
		return false;
	if (BasePlugin::canHandleMessage(s, flags))
		return true;
	if (bot()->isMyMessage(s))
		return false;

	//Stanza with already expanded alias
	if (flags.contains("glooxbot_alias") || s->hasAttribute("glooxbot_alias"))
		return false;
	return true;
}

bool AliasPlugin::parseMessage(gloox::Stanza* s, const QStringList& flags)
{
	Q_UNUSED(flags);

	MessageParser parser(s, getMyNick(s));

	QString cmd=parser.nextToken().toUpper();
	if (cmd=="ALIAS")
		return parseCommands(s);

	Alias alias=aliases.get(bot()->getStorage(s), cmd);

	if (!parser.isForMe() && !alias.isGlobal())
	{
		myShouldIgnoreError=1;
		return false;
	}

	QString res=alias.value();
	QString firstArg=res.section(' ',0,0).toUpper();

	bool noWrap=false;

	if (firstArg=="/NOWRAP")
	{
		noWrap=true;
		res=res.section(' ', 1);
	}

	if (res.contains('\n'))
	{
		//Force nowrap for multiline aliases
		noWrap=true;
	}

	if (!res.isEmpty())
	{
		while (1)
		{
			QString originalItem=res.section(";",0,0).trimmed();
			if (!noWrap)
				originalItem.replace(' ', '\n');

			if (originalItem.isEmpty())
				break;

			res=res.section(";",1).trimmed();
			QString item=expandAlias(originalItem, parser);

			if (item.isEmpty())
				continue;
			gloox::Tag *tg=s->findChild("body");
			assert(tg);
			tg->setCData(item.toStdString());
			s->addAttribute("glooxbot_alias", "true");
			s->finalize();
			bot()->processMessage(s, QStringList("glooxbot_alias"));
		}
		return true;
	}
	myShouldIgnoreError=true;
	return false;
}

bool AliasPlugin::parseCommands(gloox::Stanza* s)
{
	MessageParser parser(s, getMyNick(s), QChar(' '));
	parser.nextToken();
	QString cmd=parser.nextToken().toUpper();

	if (cmd.isEmpty() || cmd=="LIST" || cmd=="HELP")
		return false; //BasePlugin::onMessage(s);

	if (getRole(s) < ROLE_ADMIN)
	{
		reply(s, "You should be admin to edit aliases");
		return true;
	}

	if (cmd=="SHOW")
	{
		QString aliasName=parser.nextToken();
		if (aliasName.isEmpty())
		{
			QString res;
			QMap<QString, Alias> all=aliases.getAll(bot()->getStorage(s));
			int cnt=all.count();
			if (!cnt)
			{
				reply(s, "Alias list is empty");
				return true;
			}
			for (int i=0; i<cnt; i++)
				res+=QString("\n%1) %2%3=%4").arg(i+1).arg(all.values()[i].isGlobal() ? "[GLOBAL] ": "")
				.arg(all.keys()[i].toLower()).arg(all.values()[i].value());
			reply(s, QString("Aliases:%1").arg(res));
			return true;
		}
		else
		{
			Alias alias=aliases.get(bot()->getStorage(s), aliasName.toUpper());
			QString value=alias.value();
			if (!value.isNull())
				reply(s, QString("Alias: %1%2=%3").arg(alias.isGlobal() ? "[GLOBAL] " : "")
				.arg(aliasName).arg(value));
			else
				reply(s, QString("No such alias: %1").arg(aliasName));
			return true;
		}
	}
	if (cmd=="CLEAR")
	{
		aliases.clear(bot()->getStorage(s));
		reply(s, "Cleared");
		return true;
	}
	if (cmd=="COUNT")
	{
		reply(s, QString("Currently I have %1 alias(es)").arg(aliases.count(bot()->getStorage(s))));
		return true;
	}
	//arg=arg.toUpper();
	QString arg=parser.joinBody();
	QString name=arg.section('=',0,0).toUpper().trimmed();
	QString value=arg.section('=',1).trimmed();
	if (cmd=="ADD")
	{
		bool global=false;
		const QString globalStr("/global ");
		if (value.toLower().startsWith(globalStr))
		{
			global=true;
			value=value.remove(0,globalStr.length()).trimmed();
		}
		if (name.isEmpty() || value.isEmpty())
		{
			reply(s, "Syntax: !alias add <name>=<value>,"
				" where value can contain"
				" %1, %2.. - arguments; %* - all arguemnts");
			return true;
		}
		Alias alias(name, value);
		alias.setGlobal(global);
		int res=aliases.append(bot()->getStorage(s), alias);
		switch (res)
		{
		case 0:
			reply(s, "Can't store");
			break;
		case 1:
			reply(s, "Saved");
			break;
		case 2:
			reply(s, "Updated");
			break;
		}

		return true;
	}
	if (cmd=="DEL")
	{
		if (aliases.remove(bot()->getStorage(s), name))
		{
			reply(s, "Deleted");
		}
		else
		{
			reply(s, "Can't delete");
		}
		return true;
	}
	return false;
}

QString AliasPlugin::expandAlias(const QString&alias, MessageParser parser)
{
	QString res=alias;
	int idx=1;
	int parserIdx=parser.getCurrentIndex();
	while (1)
	{
		int offset=0;
		QString subStr=parser.nextToken();
		QString replacedStr=replacePattern(res, QString::number(idx), subStr,
				&offset);
		if (replacedStr==res)
			break;
		res=replacedStr;
		idx++;
		parserIdx++;
	}
	parser.setCurrentIndex(parserIdx);
	int offset=0;
	QString subStr=parser.joinBody();
	res=replacePattern(res, "\\*", subStr, &offset);
	return res.trimmed();
}

QString AliasPlugin::replacePattern(const QString& str, const QString& name,
		const QString& repl, int* offset)
{
	QString res=str;
	QRegExp exp;
	exp.setMinimal(false);
	exp.setCaseSensitivity(Qt::CaseInsensitive);
	exp.setPattern(QString("[^\\\\]\\%(\\[[^\\]]*\\]|)")+name);

	int ps=0;
	if (offset)
		ps=*offset;

	while (1)
	{
		int foundPos=exp.indexIn(res, ps);
		if (foundPos<0)
			break;
		QStringList capturedList=exp.capturedTexts();
		QString curRepl=repl;
		if (capturedList.size()==2 && !capturedList[1].isEmpty())
		{
			QString flags=capturedList[1];
			flags.remove(0, 1);
			flags.remove(flags.length()-1, 1);
			curRepl=transform(repl, flags);
		}
		res.remove(foundPos+1, exp.matchedLength()-1);
		res.insert(foundPos+1, curRepl);
		ps=foundPos+curRepl.length();
	}
	if (offset)
		*offset=ps;
	return res;
}

QString AliasPlugin::transform(const QString& str, const QString& flagsTmp)
{
	QString res=str;
	QStringList flagsList=flagsTmp.toLower().split(',');
	for (QStringList::iterator it=flagsList.begin(); it!=flagsList.end(); ++it)
	{
		QString flag=(*it);
		if (flag=="url")
			res=urlEncode(res, "UTF-8");
		else if (flag.startsWith("url:"))
		{
			QString enc=flag.section(':', 1);
			res=urlEncode(res, enc);
		}
		else if (flag=="lower")
			res=res.toLower();
		else if (flag=="upper")
			res=res.toUpper();
		else if (flag=="rev" || flag=="reverse")
		{
			QString tmp;
			int l=res.length();
			for (int i=l-1; i>=0; --i)
				tmp+=res[i];
			res=tmp;
		}
		else if (flag.startsWith("enc:"))
		{
			QString enc=flag.section(':', 1);
			QTextCodec* codec=QTextCodec::codecForName(enc.toUtf8());
			if (codec)
			{
				res=codec->fromUnicode(res);
			}
			else
			{
				qDebug() << "No codec found for " << enc;
			}
		}
		else if (flag.startsWith("enc2:"))
		{
			QString enc1=flag.section(':', 1, 1);
			QString enc2=flag.section(':', 2, 2);
			QTextCodec* codec1=QTextCodec::codecForName(enc1.toUtf8());
			QTextCodec* codec2=QTextCodec::codecForName(enc2.toUtf8());
			if (codec1 && codec2)
			{
				QByteArray arr=codec1->fromUnicode(res);
				res=codec2->toUnicode(arr);
			}
		}
		else if (flag=="blond")
		{
			QString tmp;
			int l=res.length();
			for (int i=0; i<l; ++i)
			{
				QChar ch=res[i];
				if (i%2==0)
					ch=ch.toUpper();
				else
					ch=ch.toLower();
				tmp+=ch;
			}
			res=tmp;
		}
		else if (flag.startsWith("repl:"))
		{
			QString orig=flag.section(':', 1, 1);
			QString repl=flag.section(':', 2, 2);
			res.replace(orig, repl);
		}
	}
	return res;
}
