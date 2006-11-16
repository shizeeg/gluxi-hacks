#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include "baseplugin.h"

#include <QList>

/**
	@author Dmitry Nezhevenko <dion@inhex.net>
*/
class PluginList: public QList<BasePlugin*>
{
public:
	PluginList();
	~PluginList();
	void clear();
	void remove(BasePlugin*);
// 	Plugin *byName(const QString&);
};

#endif
