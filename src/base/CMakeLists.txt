#include (${QT_USE_FILE})

INCLUDE(plugins)

set (base_HDR_MOC
	baseplugin.h
	gluxibot.h
	glooxwrapper.h
	asyncrequest.h
	asyncrequestlist.h
)

set (base_HDR
	common.h
	mystanza.h
	datastorage.h	
	pluginlist.h
	rolelist.h
	segfault.h
	messageparser.h
	vcardwrapper.h
	gloox/myvcardmanager.h
	logger.h
	config/configfield.h
	config/abstractconfigurator.h
	gluxi_version.h
)

set (base_SRC
	mystanza.cpp
	baseplugin.cpp
	common.cpp
	datastorage.cpp
	gluxibot.cpp
	glooxwrapper.cpp
	vcardwrapper.cpp
	main.cpp
	pluginlist.cpp
	asyncrequest.cpp
	asyncrequestlist.cpp
	rolelist.cpp
	messageparser.cpp
	segfault.c
	gloox/myvcardmanager.cpp
	logger.cpp
	config/configfield.cpp
	config/abstractconfigurator.cpp
	${HG_VERSION_FILE}
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR})

QT4_WRAP_CPP(base_SRC_MOC ${base_HDR_MOC})

GLUXI_CREATE_LOADER(loader_SRC GLUXI_PLUGINS)

ADD_CUSTOM_COMMAND(
	OUTPUT ${HG_VERSION_FILE}
	COMMAND ${HG_VERSION_GENERATOR} ${HG_VERSION_FILE}
	DEPENDS ${HG_VERSION_DEPENDS}
)

if(COMMAND cmake_policy)
	cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)
ADD_EXECUTABLE(gluxi ${base_SRC} ${base_SRC_MOC} ${loader_SRC})

SET (GLUXI_PLUGINS_LINK)
GLUXI_GEN_LIBS(GLUXI_PLUGINS_LIBS GLUXI_PLUGINS)

TARGET_LINK_LIBRARIES(gluxi
	${OPENSSL_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
	${QT_LIBRARIES}
	${LIBGLOOX_LIB}
	${GLUXI_PLUGINS_LIBS}
	proxy
	socketw
)
