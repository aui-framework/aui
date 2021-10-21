cmake_minimum_required(VERSION 3.10)

find_package(OpenGL)

if (OPENGL_FOUND OR ANDROID)

	auib_import(Freetype https://github.com/freetype/freetype
			    VERSION "VER-2-10-4"
				CMAKE_ARGS "-DBUILD_SHARED_LIBS=true")

	if(NOT ANDROID)
		auib_import(GLEW https://github.com/Alex2772/glew-cmake
				CMAKE_WORKING_DIR "build/cmake")

		if (NOT WIN32)
			FIND_PACKAGE(PkgConfig REQUIRED)
			PKG_CHECK_MODULES(GTK3 gtk+-3.0)

			if (NOT GTK3_FOUND)
				message(WARNING "Could not find GTK3; Disabling aui.views. Hint: install package libgtk-3-dev")
				return()
			endif()
		endif ()
	endif()

	aui_module(aui.views)
	target_include_directories(aui.views PRIVATE 3rdparty)
	if (ANDROID)
		aui_compile_assets(aui.views)
	else()
		aui_compile_assets(aui.views EXCLUDE uni/font/Roboto.ttf)
	endif()

	if(NOT ANDROID)
		if (NOT WIN32)
			include_directories (${GTK3_INCLUDE_DIRS})
			link_directories (${GTK3_LIBRARY_DIRS})
			target_link_libraries(aui.views PRIVATE gtk-3)
		endif ()
	endif()

	target_compile_definitions(aui.views PRIVATE UNICODE=1)

	target_link_libraries(aui.views PUBLIC aui.core)

	if(ANDROID)
		target_link_libraries(aui.views PRIVATE EGL GLESv2 GLESv3)
		target_link_libraries(aui.views PUBLIC Freetype::Freetype)
	else()
		target_link_libraries(aui.views PRIVATE Freetype::Freetype)
		target_link_libraries(aui.views PRIVATE OpenGL::GL)
		target_link_libraries(aui.views PUBLIC GLEW::GLEW)

		if(WIN32)
			target_link_libraries(aui.views PRIVATE dwmapi)
			target_link_libraries(aui.views PRIVATE winmm)
			if(MINGW)
				find_package(ZLIB REQUIRED)
				target_link_libraries(aui.views PRIVATE ZLIB::ZLIB)
			endif()
		endif()
	endif()
else()
	message("OpenGL library was not found. Disabling aui.views")
endif()
