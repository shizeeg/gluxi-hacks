#ifndef USERPLUGIN_H
#define USERPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class UserPlugin : public BasePlugin
{
	Q_OBJECT
public:
	UserPlugin(GluxiBot *parent = 0);
	~UserPlugin();
	virtual QString name() const { return "User"; };
	virtual QString prefix() const { return "USER"; };
	virtual QString description() const { return "Various user info commands";};
	virtual bool parseMessage(gloox::Stanza* );
	virtual bool onIq(gloox::Stanza*);
private:
	void sendVersion(gloox::Stanza *s);
};

#endif
