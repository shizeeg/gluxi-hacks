set (socketw_HDR
	src/SocketW.h
	src/sw_base.h
	src/sw_config.h
	src/sw_inet.h
	src/sw_internal.h
	src/sw_ssl.h
	src/sw_unix.h
)

set (socketw_SRC
	src/sw_base.cxx
	src/sw_inet.cxx
	src/sw_ssl.cxx
	src/sw_unix.cxx
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/plugins)

ADD_LIBRARY(socketw STATIC ${socketw_SRC})

