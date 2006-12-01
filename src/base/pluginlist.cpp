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
		delete takeAt(0);
}

void PluginList::remove(BasePlugin* plugin)
{
	removeAll(plugin);
	delete plugin;
}

