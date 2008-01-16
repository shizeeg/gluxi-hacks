#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	userplugin.h
)

set (plugin_HDR
)

set (plugin_SRC
	userplugin.cpp
	image2ascii.cpp
)

GLUXI_PLUGIN()

