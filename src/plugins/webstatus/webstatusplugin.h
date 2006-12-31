#ifndef WEBSTATUSPLUGIN_H
#define WEBSTATUSPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class WebstatusPlugin : public BasePlugin
{
	Q_OBJECT
public:
	WebstatusPlugin(GluxiBot *parent = 0);
	~WebstatusPlugin();
	virtual QString name() const { return "WebStatus"; };
	virtual QString prefix() const { return "WEBSTATUS"; };
	virtual bool parseMessage(gloox::Stanza* );
	virtual void onPresence(gloox::Stanza* s);
	virtual bool canHandlePresence(gloox::Stanza* s);
};

#endif
