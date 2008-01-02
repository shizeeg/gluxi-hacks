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
	MessageParser(const MessageParser& other);
	virtual ~MessageParser();
	QString firstToken() const;
	QString nextToken();
	void back(int count=1);
	QString joinBody();
	QString joinBody(const QChar& separator);
	bool isForMe() const { return isForMe_; }
	bool isNull() const { return null_; }
	QChar getSeparator() const { return separator_; }
	const QStringList& getTokens() const { return tokens_; }
	int getCurrentIndex() const { return currentIdx_; }

	static bool isMessageAcceptable(gloox::Stanza* st, const QString& ownNick, const QString& pluginName);
private:
	bool null_;
	bool isForMe_;
	int currentIdx_;
	QChar separator_;
	QStringList tokens_;
};

#endif /*MESSAGEPARSER_H_*/
