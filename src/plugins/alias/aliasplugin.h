#ifndef ALIASPLUGIN_H
#define ALIASPLUGIN_H

#include "base/baseplugin.h"
#include "aliaslist.h"

#include <QMap>

class MessageParser;

class AliasPlugin : public BasePlugin
{
	Q_OBJECT
public:
	AliasPlugin(GluxiBot *parent = 0);
	~AliasPlugin();
	virtual QString name() const { return "Alias"; };
	virtual QString prefix() const { return "ALIAS"; };
	virtual QString help() const { return "Alias support"; };
	virtual QString description() const { return "Can handle aliases to commands"; };
	virtual bool canHandleMessage(gloox::Stanza* s);
	virtual bool allMessages() const { return false; };

	virtual bool parseMessage(gloox::Stanza* );
	virtual bool parseCommands(gloox::Stanza* s);
private:
	AliasList aliases;
	QString replacePattern(const QString& str, const QString& name, const QString& repl, int* offset);
	QString expandAlias(const QString&alias, MessageParser parser);
	QString transform(const QString& str, const QString& flags);
};

#endif
