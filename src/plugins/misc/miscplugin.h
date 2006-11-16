#ifndef MISCPLUGIN_H
#define MISCPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class MiscPlugin : public BasePlugin
{
	Q_OBJECT
public:
	MiscPlugin(GlooxBot *parent = 0);
	~MiscPlugin();
	virtual QString name() const { return "Misc"; };
	virtual QString prefix() const { return "MISC"; };
	virtual bool parseMessage(gloox::Stanza* );
};

#endif
