#include "messageparser.h"

#include <gloox/stanza.h>

#include <QtDebug>

MessageParser::MessageParser(gloox::Stanza* st, const QString& ownNick, const QChar& separator)
{
	QString body=QString::fromStdString(st->body()).trimmed();
	int isec=0;
	isForMe_=false;
	firstEOLSeparatorTokenIdx_=-1;
	QString first=body.section(' ',isec,isec).toUpper();

	if (QString::fromStdString(st->findAttribute("type"))!="groupchat")
	{
		isForMe_=true;
	}

	if (first.startsWith('!'))
	{
		isForMe_=true;
		first.remove(0, 1);
		body.remove(0, 1);
	}
	if (!ownNick.isNull())
	{
		int l=ownNick.length();
		if (body.startsWith(ownNick) && l < body.length())
		{
			QChar ch=body.at(l);
			if (QString(":;|>").contains(ch))
			{
				body=body.mid(l+1, body.length()-l).trimmed();
				isForMe_=true;
			}
		}
	}

	if (separator==QChar() &&  body.contains('\n'))
	{
		//Using \n as token separator. Perform some "advanced" argument
		//parsing. Use \s for separator before first \n and then \n

		QString bodySpace=body.section('\n',0,0);
		QString bodyNewLine=body.section('\n',1);
		tokens_=bodySpace.split(' ');
		firstEOLSeparatorTokenIdx_=tokens_.count();
		tokens_ << bodyNewLine.split('\n');
		separator_='\n';
	}
	else
	{
		if (separator==QChar())
		{
			separator_=' ';
			tokens_=body.split(separator_);
			firstEOLSeparatorTokenIdx_=tokens_.count();
		}
		else
		{
			separator_=separator;
			tokens_=body.split(separator_);
		}

	}
	currentIdx_=0;
	null_=false;
}

MessageParser::MessageParser()
{
	null_=true;
}

MessageParser::MessageParser(const MessageParser& other)
{
	null_=other.isNull();
	isForMe_=other.isForMe();
	tokens_=other.getTokens();
	currentIdx_=other.getCurrentIndex();
	separator_=other.getSeparator();
	firstEOLSeparatorTokenIdx_=other.firstEOLSeparatorTokenIdx_;
}

MessageParser::~MessageParser()
{

}

bool MessageParser::isMessageAcceptable(gloox::Stanza* st,
		const QString& ownNick, const QString& pluginName)
{
	MessageParser parser(st, ownNick);
	return parser.isForMe() && (pluginName.isEmpty() || parser.firstToken().toUpper() ==pluginName.toUpper());
}

//Maybe this should be replaced with something like startsWith(const QString&)
//to support spaces in plugin names;
QString MessageParser::firstToken() const
{
	if (tokens_.isEmpty())
		return QString::null;
		return tokens_.first();
	}

QString MessageParser::nextToken()
{
	if (currentIdx_<tokens_.count())
	{
		return tokens_.at(currentIdx_++);
	}
	else
	{
		++currentIdx_;
		return QString::null;
	}
}

void MessageParser::back(int count)
{
	currentIdx_-=count;
	if (currentIdx_<0)
	{
		qWarning() << "MessageParser::back(): currentIdx_<0";
		currentIdx_=0;
	}
}

QString MessageParser::joinBody()
{
	return joinBody(separator_);
}

QString MessageParser::joinBody(const QChar& sep)
{
	if (currentIdx_<tokens_.count())
	{
		QString total;
		QStringList::iterator it=tokens_.begin();
		it+=currentIdx_;
		int curIdx=currentIdx_;
		bool first=true;
		while (it!=tokens_.end())
		{
			if (first)
				first=false;
			else
			{
				if (firstEOLSeparatorTokenIdx_>=0 && curIdx<firstEOLSeparatorTokenIdx_ && sep=='\n')
					total+=' ';
				else
					total+=sep;
			}

			total+=(*it);
			++it;
			++curIdx;
		}
		return total;
	}
	else
		return QString::null;
	}
