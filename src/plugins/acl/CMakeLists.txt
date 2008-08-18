#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	aclplugin.h
)

set (plugin_HDR
	acllist.h
)

set (plugin_SRC
	aclplugin.cpp
	acllist.cpp
)

GLUXI_PLUGIN()

