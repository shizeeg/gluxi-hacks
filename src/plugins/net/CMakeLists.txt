#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	netplugin.h
	pingrequest.h
)

set (plugin_HDR
)

set (plugin_SRC
	netplugin.cpp
	pingrequest.cpp
)

GLUXI_PLUGIN()

