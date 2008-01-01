#include "messageparser.h"

#include <gloox/stanza.h>

#include <QtDebug>

MessageParser::MessageParser(gloox::Stanza* st, const QString& ownNick)
{
	QString body=QString::fromStdString(st->body()).trimmed();
	int isec=0;
	isForMe_=false;
	QString first=body.section(' ',isec,isec).toUpper();
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
	//TODO handle nicks with spaces
	tokens_=body.split(' ');
	currentIdx_=0;
	null_=false;
}

MessageParser::MessageParser()
{
	null_=true;
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
		return tokens_.at(currentIdx_++);
	else
		return QString::null;
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
	if (currentIdx_<tokens_.count())
	{
		QString total;
		QStringList::iterator it=tokens_.begin();
		it+=currentIdx_;
		bool first=true;
		while (it!=tokens_.end())
		{
			if (first)
				first=false;
			else
				total+=' ';

			total+=(*it);
			++it;
		}
		return total;
	}
	else
		return QString::null;
	}
