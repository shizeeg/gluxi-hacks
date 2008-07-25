#include "pluginlist.h"
#include "baseplugin.h"

PluginList::PluginList()
{
}


PluginList::~PluginList()
{
	clear();
}

void PluginList::clear()
{
	while (count())
	{
		PluginRef ref=takeAt(0);
		BasePlugin* plugin=&(*ref);
		delete plugin;
	}
	QList<PluginRef>::clear();
}


