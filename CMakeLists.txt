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

function(AUI_Executable AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS src/*.cpp src/*.c src/*.h)
    add_executable(${AUI_MODULE_NAME} ${SRCS})

    target_include_directories(${AUI_MODULE_NAME} PUBLIC src)
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
	if (UNIX)
		target_link_libraries(${AUI_MODULE_NAME} stdc++fs)
	endif()
endfunction(AUI_Executable)
function(AUI_Module AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})
	
    file(GLOB_RECURSE SRCS src/*.cpp src/*.c src/*.h)
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

endfunction(AUI_Module)

if(MSVC)
	set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /force")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /force")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP /utf-8" )
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
add_subdirectory(AUI.Xml)
add_subdirectory(AUI.Toolbox)
add_subdirectory(tests)


#add_subdirectory(examples/AUI.Example.Sowlonya)
add_subdirectory(examples/AUI.Example.Views)
add_subdirectory(examples/AUI.Example.Words)