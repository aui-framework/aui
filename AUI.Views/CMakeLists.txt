cmake_minimum_required(VERSION 3.14)

AUI_Module(AUI.Views)

AUI_Compile_Assets(AUI.Views)

if(MSVC)
	set(GLEW_USE_STATIC_LIBS true)
endif()

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(Freetype REQUIRED)
include_directories (/usr/include/freetype2)



if (UNIX)
	FIND_PACKAGE(PkgConfig REQUIRED)
	PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

	include_directories (${GTK3_INCLUDE_DIRS})
	link_directories (${GTK3_LIBRARY_DIRS})
	target_link_libraries(AUI.Views gtk-3)
endif (UNIX)

add_definitions(-DGLEW_STATIC)

target_include_directories(AUI.Views PUBLIC ${GLEW_INCLUDE_DIRS})
target_include_directories(AUI.Views PUBLIC ${FREETYPE_INCLUDE_DIRS})

target_link_libraries(AUI.Views AUI.Core)
target_link_libraries(AUI.Views Freetype::Freetype)
target_link_libraries(AUI.Views OpenGL::GL)
if(MSVC)
	target_link_libraries(AUI.Views GLEW::glew_s)
	target_link_libraries(AUI.Views dwmapi)
	target_link_libraries(AUI.Views Winmm)
else()
	target_link_libraries(AUI.Views GLEW::glew)
endif()