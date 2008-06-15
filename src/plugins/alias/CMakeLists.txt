#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	aliasplugin.h
)

set (plugin_HDR
	alias.h
	aliaslist.h
)

set (plugin_SRC
	alias.cpp
	aliaslist.cpp
	aliasplugin.cpp
)

GLUXI_PLUGIN()

