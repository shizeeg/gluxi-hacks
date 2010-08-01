#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	miscplugin.h
	bcrequest.h
)

set (plugin_HDR
)

set (plugin_SRC
	miscplugin.cpp
	bcrequest.cpp
)

GLUXI_PLUGIN()

