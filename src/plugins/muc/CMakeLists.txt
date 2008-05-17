#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	mucplugin.h
)

set (plugin_HDR
	alist.h
	alistitem.h
	conference.h
	conferencelist.h
	jid.h
	nick.h
	nicklist.h
)

set (plugin_SRC
	alist.cpp
	alistitem.cpp
	conference.cpp
	conferencelist.cpp
	jid.cpp
	mucplugin.cpp
	nick.cpp
	nicklist.cpp
)

GLUXI_PLUGIN()


