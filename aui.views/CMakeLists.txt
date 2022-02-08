cmake_minimum_required(VERSION 3.10)


set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
find_package(OpenGL)
if (AUI_PLATFORM_LINUX)
	find_package(X11 REQUIRED)
	if(BUILD_SHARED_LIBS)
		find_package(Fontconfig REQUIRED)
	endif()
endif()
set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)

if (OPENGL_FOUND OR ANDROID OR IOS)

	auib_import(Freetype https://github.com/aui-framework/freetype
			    CMAKE_ARGS -DFT_DISABLE_HARFBUZZ=TRUE
			               -DFT_DISABLE_PNG=TRUE
			               -DFT_DISABLE_ZLIB=TRUE
			               -DFT_DISABLE_BZIP2=TRUE
			               -DFT_DISABLE_BROTLI=TRUE)


	if(NOT ANDROID AND NOT IOS)
		auib_import(GLEW https://github.com/aui-framework/glew-cmake
				CMAKE_WORKING_DIR "build/cmake")

		if (NOT WIN32 AND NOT APPLE)
			FIND_PACKAGE(PkgConfig REQUIRED)
			PKG_CHECK_MODULES(GTK gtk+-3.0)

			if (NOT GTK_FOUND)
				message(WARNING "Could not find GTK3; Disabling aui.views. Hint: install package libgtk-3-dev")
				return()
			endif()
			link_directories(${GTK_LIBRARY_DIRS})
			add_definitions(${GTK_CFLAGS_OTHER})
		endif ()
	endif()

	aui_module(aui.views EXPORT aui)

	if (AUI_PLATFORM_LINUX)
		target_include_directories(aui.views PRIVATE ${GTK_INCLUDE_DIRS})
		aui_link(aui.views PRIVATE X11::X11 X11::Xrandr X11::Xcursor X11::Xi ${GTK_LIBRARIES})
		if (NOT BUILD_SHARED_LIBS)
			# link Freetype's deps
			target_link_libraries(aui.views PUBLIC ${Fontconfig_LIBRARIES})
		endif()
	endif()

	if (NOT AUI_PLATFORM_WINDOWS)
		aui_compile_assets(aui.views)
	else()
		aui_compile_assets(aui.views EXCLUDE uni/font/Roboto.ttf)
	endif()

	target_compile_definitions(aui.views PRIVATE UNICODE=1)

	aui_link(aui.views PUBLIC aui::core aui::image)
	aui_link(aui.views PRIVATE aui::xml)
	set_target_properties(Freetype::Freetype PROPERTIES INSTALL_NAME_DIR "@rpath" BUILD_WITH_INSTALL_RPATH 1)
	if(ANDROID)
		aui_link(aui.views PRIVATE EGL GLESv2 GLESv3)
		aui_link(aui.views PRIVATE Freetype::Freetype)
	elseif(IOS)
		set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
		find_library(OPENGL OpenGLES REQUIRED)
		set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)
		aui_link(aui.views PRIVATE ${OPENGL})
		aui_link(aui.views PUBLIC Freetype::Freetype)
	else()
		aui_link(aui.views PRIVATE Freetype::Freetype)
		aui_link(aui.views PRIVATE OpenGL::GL)
		aui_link(aui.views PUBLIC GLEW::GLEW)

		if(WIN32)
			aui_link(aui.views PRIVATE dwmapi)
			aui_link(aui.views PRIVATE winmm)
			if(MINGW)
				find_package(ZLIB REQUIRED)
				aui_link(aui.views PRIVATE ZLIB::ZLIB)
			endif()
		elseif(AUI_PLATFORM_MACOS)
			target_link_libraries(aui.views PUBLIC "-framework AppKit"
				                                   "-framework Cocoa"
				                                   "-framework CoreData"
				                                   "-framework Foundation"
				                                   "-framework QuartzCore")
		endif()
	endif()
else()
	message("OpenGL library was not found. Disabling aui.views")
endif()
