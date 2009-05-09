#ifndef USERPLUGIN_H
#define USERPLUGIN_H

#include "base/baseplugin.h"

#include <gloox/stanza.h>
#include <gloox/vcardhandler.h>

class VCardWrapper;

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
	virtual bool canHandleIq(gloox::Stanza*);
	virtual bool onIq(gloox::Stanza*);
	virtual bool onVCard(const VCardWrapper& vcard);
private:
	void sendVersion(gloox::Stanza *s);
	void sendTime(gloox::Stanza* s);
	QString utcToString(const QString &cdata, const QString &format);
	QString resolveTargetJid(gloox::Stanza* s, const QString& arg);
};

#endif
