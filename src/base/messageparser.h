#ifndef MESSAGEPARSER_H_
#define MESSAGEPARSER_H_

#include <QStringList>

namespace gloox 
{
	class Stanza;
}

class MessageParser
{
public:
	MessageParser();
	MessageParser(gloox::Stanza* st, const QString& ownNick=QString());
	virtual ~MessageParser();
	QString firstToken() const;
	QString nextToken();
	void back(int count=1);
	QString joinBody();
	bool isForMe() const { return isForMe_; }
	bool isNull() const { return null_; }

	static bool isMessageAcceptable(gloox::Stanza* st, const QString& ownNick, const QString& pluginName);
private:
	bool null_;
	bool isForMe_;
	int currentIdx_;
	QStringList tokens_;
};

#endif /*MESSAGEPARSER_H_*/
