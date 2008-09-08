#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	rosterplugin.h
)

set (plugin_HDR
	rosterstorage.h
)

set (plugin_SRC
	rosterplugin.cpp
	rosterstorage.cpp
)

GLUXI_PLUGIN()

