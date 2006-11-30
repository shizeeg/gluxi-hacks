MACRO (GLUXI_CLEAR_CPP)
	FILE(WRITE "${PROJECT_BINARY_DIR}/src/pluginloader.cpp" "")
ENDMACRO(GLUXI_CLEAR_CPP)

MACRO (GLUXI_APPEND_CPP _msg)
	FILE(APPEND "${PROJECT_BINARY_DIR}/src/pluginloader.cpp" "${_msg}\n")
ENDMACRO(GLUXI_APPEND_CPP)

MACRO (GLUXI_PLUGIN)
	STRING(REGEX MATCH "[^/]*$" _name ${CMAKE_CURRENT_SOURCE_DIR})
	# Add plugin to global plugin list
	SET(GLUXI_PLUGIN_LIST
		${GLUXI_PLUGIN_LIST}
		${_name}
	)
	SET(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/build/plugins)
	
	QT4_WRAP_CPP(plugin_SRC_MOC ${plugin_HDR_MOC})

	ADD_LIBRARY(gluxi_plugin_${_name} STATIC ${plugin_SRC} ${plugin_SRC_MOC})

ENDMACRO (GLUXI_PLUGIN)

MACRO (GLUXI_FIND_PLUGINS _allplugins _dir)
	SET(_out)
	MESSAGE(STATUS "Finding plugins in ${_dir}")
	STRING (LENGTH ${_dir} _length)
	FILE (GLOB _plugin_list ${_dir}/*)
        FOREACH(_plugin ${_plugin_list})
                FILE(GLOB _cmake ${_plugin}/CMakeLists.txt)
                IF(_cmake)

                        STRING(LENGTH ${_plugin} _plugin_length)
                        SET (_name_length ${_plugin_length}-${_length})
                        STRING (REGEX MATCH "[^/]*$" _name ${_plugin})
                        MESSAGE(STATUS "-> Found plugin: ${_name}")
			SET (_out
				${_out}
				${_name}
			)	
                ENDIF(_cmake)
        ENDFOREACH(_plugin)
	SET(${_allplugins} ${_out})
ENDMACRO(GLUXI_FIND_PLUGINS)

MACRO (GLUXI_UPCASE _out _in)
	STRING(LENGTH ${_in} _length)
	MATH(EXPR _length1 "${_length}-1")
	STRING(SUBSTRING ${_in} 0 1 _first)
	STRING(TOUPPER ${_first} _firstU)
	STRING(SUBSTRING ${_in} 1 ${_length1} _other)
	SET(_tmp "${_firstU}${_other}")
	SET(${_out} ${_tmp})
ENDMACRO(GLUXI_UPCASE)

MACRO (GLUXI_CREATE_LOADER _out _list)
	GLUXI_CLEAR_CPP()
	GLUXI_APPEND_CPP("// Don't edit this file\n")
	GLUXI_APPEND_CPP("#include \"base/pluginloader.h\"")
	GLUXI_APPEND_CPP("#include \"base/pluginlist.h\"")
	GLUXI_APPEND_CPP("#include \"base/gluxibot.h\"")
	GLUXI_APPEND_CPP("#include \"base/baseplugin.h\"")
	FOREACH(_plugin ${${_list}})
		GLUXI_APPEND_CPP("#include \"plugins/${_plugin}/${_plugin}plugin.h\"")
	ENDFOREACH(_plugin)
	GLUXI_APPEND_CPP("")
	GLUXI_APPEND_CPP("void PluginLoader::loadPlugins(PluginList* lst, GluxiBot *parent)")
	GLUXI_APPEND_CPP("{")
	FOREACH(_plugin ${${_list}})
		GLUXI_UPCASE(_up ${_plugin})
		GLUXI_APPEND_CPP("	lst->append(new ${_up}Plugin(parent));")
	ENDFOREACH(_plugin)
	GLUXI_APPEND_CPP("}\n")
	MESSAGE(STATUS "PluginLoader generated")
	INCLUDE_DIRECTORIES(${PROJECT_BINARY_DIR})
	SET(${_out} "${PROJECT_BINARY_DIR}/src/pluginloader.cpp")
ENDMACRO(GLUXI_CREATE_LOADER)


MACRO (GLUXI_ALL_PLUGINS _list)
#	GLUXI_CREATE_LOADER(${_list})
	FOREACH(_plugin ${${_list}})
		ADD_SUBDIRECTORY(${_plugin})
	ENDFOREACH(_plugin)
ENDMACRO (GLUXI_ALL_PLUGINS)

MACRO (GLUXI_GEN_LIBS _out _list)
	SET (__out)
	FOREACH(_plugin ${${_list}})
		SET (__out
			${__out}
			gluxi_plugin_${_plugin}
		)
	ENDFOREACH(_plugin)
	SET(${_out} ${__out})
ENDMACRO(GLUXI_GEN_LIBS)

