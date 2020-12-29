cmake_minimum_required(VERSION 3.16)

project(AUI)


define_property(GLOBAL PROPERTY TESTS_SRCS
		BRIEF_DOCS "Global list of test files"
		FULL_DOCS "Global list of test files")
define_property(GLOBAL PROPERTY TESTS_DEPS
		BRIEF_DOCS "Global list of test dependencies"
		FULL_DOCS "Global list of test dependencies")

set_property(GLOBAL PROPERTY TESTS_SRCS "")


include(cmake/b.cmake)

function(AUI_Module AUI_MODULE_NAME)
	project(${AUI_MODULE_NAME})

	file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.manifest src/*.h src/*.hpp)
	if (WIN32)
		set(SRCS ${SRCS} "${CMAKE_SOURCE_DIR}/Resource.rc")
	endif()
	add_library(${AUI_MODULE_NAME} SHARED ${SRCS} ${ARGN})
	get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
	target_include_directories(${AUI_MODULE_NAME} PUBLIC $<BUILD_INTERFACE:${SELF_DIR}/src>)

	# AUI.Core -> BUILD_AUI_CORE
	string(REPLACE "." "_" BUILD_DEF_NAME ${AUI_MODULE_NAME})
	string(TOUPPER "API_${BUILD_DEF_NAME}" BUILD_DEF_NAME)
	target_compile_definitions(${AUI_MODULE_NAME} INTERFACE ${BUILD_DEF_NAME}=AUI_IMPORT)
	target_compile_definitions(${AUI_MODULE_NAME} PRIVATE ${BUILD_DEF_NAME}=AUI_EXPORT)

	AUI_Add_Properties(${AUI_MODULE_NAME})

	AUI_Common(${AUI_MODULE_NAME})

	install(
			TARGETS ${AUI_MODULE_NAME}
			EXPORT AUI
			ARCHIVE
				DESTINATION "lib"
			LIBRARY
				DESTINATION "lib"
			RUNTIME
				DESTINATION "bin"
	)
	install(
			DIRECTORY src/
			DESTINATION "include/"
			FILES_MATCHING PATTERN "*.h"
			PATTERN "*.hpp"

	)

endfunction(AUI_Module)

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


if(NOT ANDROID)
	#add_subdirectory(examples/AUI.Example.Sowlonya)
	add_subdirectory(examples/AUI.Example.Views)
	#add_subdirectory(examples/AUI.Example.Notes)
	#add_subdirectory(examples/AUI.Example.Words)
	#add_subdirectory(examples/AUI.Example.Fractal)
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
install(EXPORT AUI DESTINATION "cmake/")

