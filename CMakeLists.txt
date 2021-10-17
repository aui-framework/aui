# ======================================================================================================================
# Copyright (c) 2021 Alex2772
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/aui-framework/aui
# ======================================================================================================================

cmake_minimum_required(VERSION 3.16)

project(AUI)

if(AUI_BOOT_COMPONENTS)
	# adding required dependencies to the components
	list(APPEND AUI_BOOT_COMPONENTS core)
	if (views IN_LIST AUI_BOOT_COMPONENTS)
		list(APPEND AUI_BOOT_COMPONENTS crypt toolbox)
	endif()
endif()
list(REMOVE_DUPLICATES AUI_BOOT_COMPONENTS)

include(aui.boot.cmake)

# use msvc dll
cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")

define_property(GLOBAL PROPERTY TESTS_SRCS
		BRIEF_DOCS "Global list of test files"
		FULL_DOCS "Global list of test files")
define_property(GLOBAL PROPERTY TESTS_DEPS
		BRIEF_DOCS "Global list of test dependencies"
		FULL_DOCS "Global list of test dependencies")

set_property(GLOBAL PROPERTY TESTS_SRCS "")

set(AUI_BUILD_EXAMPLES false CACHE BOOL "Build AUI examples")


include(cmake/b.cmake)

function(define_aui_component AUI_COMPONENT_NAME)
	if (AUI_BOOT_COMPONENTS)
		if (NOT ${AUI_COMPONENT_NAME} IN_LIST AUI_BOOT_COMPONENTS)
			return()
		endif()
		message(STATUS "[+] ${AUI_COMPONENT_NAME}")
	endif()
	add_subdirectory("aui.${AUI_COMPONENT_NAME}")
endfunction()

define_aui_component(core)
define_aui_component(crypt)
define_aui_component(toolbox)
define_aui_component(curl)
define_aui_component(data)
define_aui_component(json)
define_aui_component(mysql)
define_aui_component(sqlite)
define_aui_component(network)
define_aui_component(xml)

define_aui_component(views)
define_aui_component(svg)
define_aui_component(image)

if(NOT ANDROID AND AUI_BUILD_EXAMPLES)
	add_subdirectory(examples/AUI.Example.Views)
	#add_subdirectory(examples/AUI.Example.Fractal)
	#add_subdirectory(examples/AUI.Example.Minesweeper)
endif()

get_property(TESTS_SRCS GLOBAL PROPERTY TESTS_SRCS)

if (NOT ANDROID)
	aui_tests(Tests ${TESTS_SRCS})
	if (TARGET Tests)
		get_property(TESTS_DEPS GLOBAL PROPERTY TESTS_DEPS)
		foreach(child ${TESTS_DEPS})
			target_link_libraries(Tests PRIVATE ${child})
		endforeach()
	endif()
endif()

#install(EXPORT AUI DESTINATION "cmake/")
set(AUI_ALL_COMPONENTS core
		crypt
		curl
		data
		image
		json
		mysql
		network
		sqlite
		views
		xml
		)

if(NOT AUI_BOOT_COMPONENTS)
	# adding all components
	set(AUI_BOOT_COMPONENTS ${AUI_ALL_COMPONENTS})

	# remove unexisting modules from module list (i.e. toolbox)
	foreach(_module ${AUI_BOOT_COMPONENTS})
		if (NOT TARGET "aui.${_module}")
			list(REMOVE_ITEM AUI_BOOT_COMPONENTS ${_module})
		endif()
	endforeach()
else()
	# remove executables from module list (i.e. toolbox)
	foreach(_module ${AUI_BOOT_COMPONENTS})
		get_target_property(_type "aui.${_module}" TYPE)
		if (_type STREQUAL "EXECUTABLE")
			list(REMOVE_ITEM AUI_BOOT_COMPONENTS ${_module})
		endif()
	endforeach()
endif()

set(AUI_CONFIG_VARS "set(AUI_ALL_COMPONENTS ${AUI_ALL_COMPONENTS})\nset(AUI_AVAILABLE_COMPONENTS ${AUI_BOOT_COMPONENTS})\n")
get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
# forward AUI's dependencies
set(AUI_CONFIG_VARS "${AUI_CONFIG_VARS}\nset(AUI_BOOT_ROOT_ENTRIES ${AUI_BOOT_ROOT_ENTRIES})\n")
foreach(_module ${AUI_BOOT_COMPONENTS})
	set(_target "aui.${_module}")
	get_target_property(_compile_definitions ${_target} INTERFACE_COMPILE_DEFINITIONS)
	get_target_property(_link_libs ${_target} INTERFACE_LINK_LIBRARIES)
	unset(_link_libs2)
	foreach(_lib ${_link_libs})
		string(REPLACE "aui." "" _lib ${_lib})
		list(APPEND _link_libs2 ${_lib})
	endforeach()
	set(AUI_CONFIG_VARS "${AUI_CONFIG_VARS}\

set(AUI_COMPONENT_${_module}_COMPILE_DEFINITIONS ${_compile_definitions})
set(AUI_COMPONENT_${_module}_LINK_LIBS ${_link_libs2})
")
endforeach()


configure_file(cmake/c.cmake ${CMAKE_CURRENT_BINARY_DIR}/c.cmake @ONLY)
#install(EXPORT AUI DESTINATION cmake/)
install(
		FILES
		${CMAKE_CURRENT_BINARY_DIR}/c.cmake
		RENAME aui-config.cmake
		DESTINATION ".")

install(
		FILES
			cmake/b.cmake
		RENAME AUI.Build.cmake
		DESTINATION "cmake/")


