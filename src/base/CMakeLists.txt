#include (${QT_USE_FILE})

INCLUDE(plugins)

set (base_HDR_MOC
	baseplugin.h
	glooxbot.h
)

set (base_HDR
	common.h
	datastorage.h
	glooxbot.h
	pluginlist.h
)

set (base_SRC
	baseplugin.cpp
	common.cpp
	datastorage.cpp
	glooxbot.cpp
	main.cpp
	pluginlist.cpp
)

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build)

QT4_WRAP_CPP(base_SRC_MOC ${base_HDR_MOC})

GLUXI_CREATE_LOADER(loader_SRC GLUXI_PLUGINS)

ADD_EXECUTABLE(gluxi ${base_SRC} ${base_SRC_MOC} ${loader_SRC})

SET (GLUXI_PLUGINS_LINK)
GLUXI_GEN_LIBS(GLUXI_PLUGINS_LIBS GLUXI_PLUGINS)

TARGET_LINK_LIBRARIES(gluxi 
	${QT_LIBRARIES}
	${LIBGLOOX_LIB}
	${GLUXI_PLUGINS_LIBS}
)


