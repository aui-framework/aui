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

if (AUI_BOOT)
	# install fake config
	set(AUI_SOURCE_LOCATION ${CMAKE_CURRENT_SOURCE_DIR})
	configure_file(cmake/fake-config.cmake fake-config.cmake @ONLY)
	install(
			FILES
			${CMAKE_BINARY_DIR}/fake-config.cmake
			RENAME aui-config.cmake
			DESTINATION ".")
else()
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

	add_subdirectory(aui.core)
	add_subdirectory(aui.crypt)
	add_subdirectory(aui.toolbox)
	add_subdirectory(aui.curl)
	add_subdirectory(aui.data)
	add_subdirectory(aui.json)
	add_subdirectory(AUI.Mysql)
	add_subdirectory(aui.sqlite)
	add_subdirectory(aui.network)
	add_subdirectory(aui.xml)

	add_subdirectory(aui.views)
	add_subdirectory(aui.svg)
	add_subdirectory(aui.image)

	add_subdirectory(aui.preview)

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
	install(
			FILES
			cmake/c.cmake
			RENAME aui-config.cmake
			DESTINATION ".")
endif()

install(
		FILES
			cmake/b.cmake
		RENAME AUI.Build.cmake
		DESTINATION "cmake/")


