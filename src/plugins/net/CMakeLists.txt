#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	netplugin.h
	pingrequest.h
	tracerouterequest.h
	wwwrequest.h
	xeprequest.h
	googlerequest.h
	svnrequest.h
	currencyrequest.h
	translaterequest.h
)

set (plugin_HDR
)

set (plugin_SRC
	netplugin.cpp
	pingrequest.cpp
	tracerouterequest.cpp
	wwwrequest.cpp
	xeprequest.cpp
	googlerequest.cpp
	svnrequest.cpp
	currencyrequest.cpp
	translaterequest.cpp
)

GLUXI_PLUGIN()

