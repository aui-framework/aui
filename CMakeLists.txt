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
# Original code located at https://github.com/Alex2772/aui
# ======================================================================================================================

cmake_minimum_required(VERSION 3.16)

project(AUI)

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

add_subdirectory(AUI.Core)
add_subdirectory(AUI.Crypt)
add_subdirectory(AUI.Toolbox)
add_subdirectory(AUI.Curl)
add_subdirectory(AUI.Data)
add_subdirectory(AUI.Json)
add_subdirectory(AUI.Mysql)
add_subdirectory(AUI.Sqlite)
add_subdirectory(AUI.Network)
add_subdirectory(AUI.Xml)

add_subdirectory(AUI.Views)
add_subdirectory(AUI.Svg)
add_subdirectory(AUI.Image)

add_subdirectory(AUI.Preview)

if(NOT ANDROID AND AUI_BUILD_EXAMPLES)
	add_subdirectory(examples/AUI.Example.Views)
	#add_subdirectory(examples/AUI.Example.Fractal)
	#add_subdirectory(examples/AUI.Example.Minesweeper)
endif()

get_property(TESTS_SRCS GLOBAL PROPERTY TESTS_SRCS)

if (NOT ANDROID)
	AUI_Tests(Tests ${TESTS_SRCS})
	if (TARGET Tests)
		get_property(TESTS_DEPS GLOBAL PROPERTY TESTS_DEPS)
		foreach(child ${TESTS_DEPS})
			target_link_libraries(Tests PRIVATE ${child})
		endforeach()
	endif()
endif()


## INSTALL
install(
		FILES
			cmake/c.cmake
		RENAME aui-config.cmake
		DESTINATION ".")
install(
		FILES
			cmake/b.cmake
		RENAME AUI.Build.cmake
		DESTINATION "cmake/")

# TODO broken
# install(EXPORT AUI DESTINATION "cmake/")

