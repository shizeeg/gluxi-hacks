#ifndef COREPLUGIN_H
#define COREPLUGIN_H

#include "base/baseplugin.h"

class CorePlugin : public BasePlugin
{
	Q_OBJECT
public:
	CorePlugin(GlooxBot *parent = 0);
	~CorePlugin();
	virtual QString name() const { return QString::null; };
	virtual QString prefix() const { return QString::null; };
	virtual QString help() const { return QString::null; };
	virtual bool onMessage(gloox::Stanza* );
};

#endif
