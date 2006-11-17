#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	wordplugin.h
)

set (plugin_HDR
	wordlist.h
)

set (plugin_SRC
	wordplugin.cpp
	wordlist.cpp
)

GLUXI_PLUGIN()

