#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	mucplugin.h
	nickasyncrequest.h
)

set (plugin_HDR
	alist.h
	alistitem.h
	conference.h
	conferencelist.h
	jid.h
	jidstat.h
	jidtimestat.h
	muchistory.h
	nick.h
	nicklist.h
	config/mucconfigurator.h
)

set (plugin_SRC
	alist.cpp
	alistitem.cpp
	conference.cpp
	conferencelist.cpp
	jid.cpp
	jidstat.cpp
	jidtimestat.cpp
	muchistory.cpp
	mucplugin.cpp
	nick.cpp
	nicklist.cpp
	config/mucconfigurator.cpp
	nickasyncrequest.cpp
)

GLUXI_PLUGIN()


