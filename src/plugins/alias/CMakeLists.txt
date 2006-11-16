#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	aliasplugin.h
)

set (plugin_HDR
	aliaslist.h
)

set (plugin_SRC
	aliaslist.cpp
	aliasplugin.cpp
)

GLUXI_PLUGIN()

