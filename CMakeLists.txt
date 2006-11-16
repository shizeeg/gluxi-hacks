PROJECT(gluxi)

CMAKE_MINIMUM_REQUIRED(VERSION 2.4.0)

SET(CMAKE_COLOR_MAKEFILE ON)
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

set (LIB_SEARCH
	/usr/lib
	/usr/local/lib
)

set (INC_SEARCH
	/usr/include
	/usr/local/include
)


# Find Qt4
SET (QT_DONT_USE_QTGUI TRUE)
SET (QT_USE_QTSQL TRUE)
find_package(Qt4 REQUIRED)

include(${QT_USE_FILE})

# Find libgloox library
find_library(LIBGLOOX_LIB NAMES gloox PATHS ${LIB_SEARCH})
find_path(LIBGLOOX_INC gloox/gloox.h ${INC_SEARCH})

INCLUDE_DIRECTORIES(
	${INCLUDE_DIRECTORIES}
	${QT_INCLUDE_DIR}
	${QT_QTSQL_INCLUDE_DIR}
	${LIBGLOOX_INC}
)

add_subdirectory(src)

