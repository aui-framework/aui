cmake_minimum_required(VERSION 3.14)

project(AUI)

ADD_DEFINITIONS(-DUNICODE)

set(AUI_3RDPARTY_LIBS_DIR NOTFOUND CACHE PATH "")
if (AUI_3RDPARTY_LIBS_DIR)
	FILE(GLOB children RELATIVE ${AUI_3RDPARTY_LIBS_DIR} ${AUI_3RDPARTY_LIBS_DIR}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${AUI_3RDPARTY_LIBS_DIR}/${child})
			list(APPEND CMAKE_PREFIX_PATH ${AUI_3RDPARTY_LIBS_DIR}/${child})
		endif()
	endforeach()
	message("LIBRARY PATH: ${CMAKE_PREFIX_PATH}")
endif()

function(AUI_Add_Properties AUI_MODULE_NAME)
	if(MSVC)
		set_target_properties(${AUI_MODULE_NAME} PROPERTIES
				LINK_FLAGS "/force:MULTIPLE"
				COMPILE_FLAGS "/MP /utf-8")
	endif()

	set_target_properties(${AUI_MODULE_NAME} PROPERTIES
			ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
			LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
			RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

endfunction(AUI_Add_Properties)

function(AUI_Executable AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    add_executable(${AUI_MODULE_NAME} ${SRCS})

    target_include_directories(${AUI_MODULE_NAME} PUBLIC src)
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
	if (UNIX)
		target_link_libraries(${AUI_MODULE_NAME} stdc++fs)
	endif()

	AUI_Add_Properties(${AUI_MODULE_NAME})
endfunction(AUI_Executable)

function(AUI_Module AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})
	
    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)
    add_library(${AUI_MODULE_NAME} SHARED ${SRCS} ${ARGN})

    target_include_directories(${AUI_MODULE_NAME} PUBLIC src)
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
	if (UNIX)
		target_link_libraries(${AUI_MODULE_NAME} stdc++fs)
	endif()

	# AUI.Core -> BUILD_AUI_CORE
	string(REPLACE "." "_" BUILD_DEF_NAME ${AUI_MODULE_NAME})
	string(TOUPPER "API_${BUILD_DEF_NAME}" BUILD_DEF_NAME)
	target_compile_definitions(${AUI_MODULE_NAME} INTERFACE ${BUILD_DEF_NAME}=AUI_IMPORT)
	target_compile_definitions(${AUI_MODULE_NAME} PRIVATE ${BUILD_DEF_NAME}=AUI_EXPORT)

	AUI_Add_Properties(${AUI_MODULE_NAME})
endfunction(AUI_Module)

function(AUI_Compile_Assets AUI_MODULE_NAME)
	set(TARGET_NAME "${AUI_MODULE_NAME}.Assets")

	get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
	set(CMD_STRING ${TARGET_DIR}/AUI.Toolbox pack ${CMAKE_CURRENT_SOURCE_DIR}/assets ${CMAKE_CURRENT_BINARY_DIR}/autogen/assets.cpp)
	add_custom_target(${TARGET_NAME} COMMAND ${CMD_STRING})
	add_dependencies(${TARGET_NAME} AUI.Toolbox)
	add_dependencies(${AUI_MODULE_NAME} ${TARGET_NAME})
endfunction(AUI_Compile_Assets)

add_subdirectory(AUI.Core)
add_subdirectory(AUI.Crypt)
add_subdirectory(AUI.Curl)
add_subdirectory(AUI.Data)
add_subdirectory(AUI.Json)
add_subdirectory(AUI.Mysql)
add_subdirectory(AUI.Sqlite)
add_subdirectory(AUI.Network)
add_subdirectory(AUI.Views)
add_subdirectory(AUI.Svg)
add_subdirectory(AUI.Image)
add_subdirectory(AUI.Xml)
add_subdirectory(AUI.Toolbox)
add_subdirectory(tests)


#add_subdirectory(examples/AUI.Example.Sowlonya)
add_subdirectory(examples/AUI.Example.Views)
add_subdirectory(examples/AUI.Example.Words)