cmake_minimum_required(VERSION 3.10)

find_package(OpenGL)

if (OPENGL_FOUND)

	auib_import(Freetype https://github.com/freetype/freetype
			    VERSION "VER-2-10-4"
				CMAKE_ARGS "-DBUILD_SHARED_LIBS=true")
	auib_import(GLEW https://github.com/Alex2772/glew-cmake
				CMAKE_WORKING_DIR "build/cmake")


	if(NOT ANDROID)
		if (NOT WIN32)
			FIND_PACKAGE(PkgConfig REQUIRED)
			PKG_CHECK_MODULES(GTK3 gtk+-3.0)

			if (NOT GTK3_FOUND)
				message(WARNING "Could not find GTK3; Disabling AUI.Views. Hint: install package libgtk-3-dev")
				return()
			endif()
		endif ()
	endif()

	aui_module(AUI.Views)

	aui_compile_assets(AUI.Views)

	if(NOT ANDROID)
		if (NOT WIN32)
			include_directories (${GTK3_INCLUDE_DIRS})
			link_directories (${GTK3_LIBRARY_DIRS})
			target_link_libraries(AUI.Views PRIVATE gtk-3)
		endif ()
	endif()

	target_compile_definitions(AUI.Views PRIVATE UNICODE=1)

	target_link_libraries(AUI.Views PUBLIC AUI.Core GLEW::GLEW)

	if(ANDROID)
		target_link_libraries(AUI.Views PUBLIC EGL GLESv2 GLESv3)
		target_link_libraries(AUI.Views PUBLIC freetype)
	else()
		target_link_libraries(AUI.Views PUBLIC OpenGL::GL)

		if (FREETYPE_FOUND)
			target_link_libraries(AUI.Views PRIVATE Freetype::Freetype)
		else()
			target_link_libraries(AUI.Views PRIVATE freetype)
		endif()
		if(WIN32)
			if (GLEW_FOUND)
				target_link_libraries(AUI.Views PUBLIC GLEW::glew_s)
			endif()
			target_link_libraries(AUI.Views PRIVATE dwmapi)
			target_link_libraries(AUI.Views PRIVATE winmm)
			if(MINGW)
				find_package(ZLIB REQUIRED)
				target_link_libraries(AUI.Views PRIVATE ZLIB::ZLIB)
			endif()
		else()
			if (GLEW_FOUND)
				target_link_libraries(AUI.Views PUBLIC GLEW::glew)
			endif()
		endif()
		if (NOT GLEW_FOUND)
			# fix install error for glew_s
			target_link_libraries(AUI.Views PUBLIC glew_s)

			target_include_directories(AUI.Views PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glew/include>)
		endif()
	endif()
else()
	message("OpenGL library was not found. Disabling AUI.Views")
endif()
