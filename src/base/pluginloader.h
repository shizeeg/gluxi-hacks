#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

class PluginList;
class GlooxBot;

class PluginLoader
{
public:
	static void loadPlugins(PluginList* lst, GlooxBot* bot);
};

#endif

