#ifndef PLUGINLIST_H
#define PLUGINLIST_H

#include <QList>

class BasePlugin;

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
