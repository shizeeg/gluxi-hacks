#include (${QT_USE_FILE})

set (plugin_HDR_MOC
	netplugin.h
	pingrequest.h
	tracerouterequest.h
	wwwrequest.h
	xeprequest.h
)

set (plugin_HDR
)

set (plugin_SRC
	netplugin.cpp
	pingrequest.cpp
	tracerouterequest.cpp
	wwwrequest.cpp
	xeprequest.cpp
)

GLUXI_PLUGIN()

