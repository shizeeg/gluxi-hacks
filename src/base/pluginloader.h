#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

class PluginList;
class GluxiBot;

class PluginLoader
{
public:
	static void loadPlugins(PluginList* lst, GluxiBot* bot);
};

#endif

