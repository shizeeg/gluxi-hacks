#ifndef NETPLUGIN_H
#define NETPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class NetPlugin : public BasePlugin
{
	Q_OBJECT
public:
	NetPlugin(GluxiBot *parent = 0);
	~NetPlugin();
	virtual QString name() const { return "Net"; };
	virtual QString prefix() const { return "NET"; };
	virtual bool parseMessage(gloox::Stanza*, const QStringList& flags );
};

#endif
