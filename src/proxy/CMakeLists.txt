set (proxy_HDR
	baseproxy.h
	directproxy.h
)

set (proxy_SRC
	baseproxy.cpp
	directproxy.cpp
)

SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/plugins)

ADD_LIBRARY(proxy STATIC ${proxy_SRC})

