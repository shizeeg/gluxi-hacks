#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	webstatusplugin.h
	webstatusthread.h
)

set (plugin_HDR
)

set (plugin_SRC
	webstatusplugin.cpp
	webstatusthread.cpp
)

GLUXI_PLUGIN()

