#ifndef WORDPLUGIN_H
#define WORDPLUGIN_H

#include "wordlist.h"
#include "base/baseplugin.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class WordPlugin : public BasePlugin
{
	Q_OBJECT
public:
	WordPlugin(GluxiBot *parent = 0);
	~WordPlugin();
	virtual QString name() const { return "Word"; };
	virtual QString prefix() const { return "WORD"; };
	virtual bool parseMessage(gloox::Stanza*, const QStringList& flags);
private:
	WordList words;
};

#endif

