#ifndef ADMINPLUGIN_H
#define ADMINPLUGIN_H

#include "base/baseplugin.h"

class AdminPlugin : public BasePlugin
{
	Q_OBJECT
public:
	AdminPlugin(GlooxBot *parent = 0);
	~AdminPlugin();
	virtual QString name() const { return "Admin"; };
	virtual QString prefix() const { return "ADMIN"; };
	virtual QString help() const { return "This plugin contains bot administrator tools"; };
	virtual QString description() const { return "Bot administrator tools";};
	virtual bool parseMessage(gloox::Stanza* );
};

#endif
