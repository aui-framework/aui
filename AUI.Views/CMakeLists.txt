cmake_minimum_required(VERSION 3.10)

if (OPENGL_FOUND)

	if(NOT ANDROID)
		include_directories (/usr/include/freetype2)

		if (NOT WIN32)
			FIND_PACKAGE(PkgConfig REQUIRED)
			PKG_CHECK_MODULES(GTK3 gtk+-3.0)

			if (NOT GTK3_FOUND)
				message(WARNING "Could not find GTK3; Disabling AUI.Views. Hint: install package libgtk-3-dev")
				return()
			endif()
		endif ()
	endif()

	AUI_Module(AUI.Views)

	AUI_Compile_Assets(AUI.Views)


	find_package(Freetype)

	if(NOT ANDROID)
		include_directories (/usr/include/freetype2)

		if (NOT WIN32)
			include_directories (${GTK3_INCLUDE_DIRS})
			link_directories (${GTK3_LIBRARY_DIRS})
			target_link_libraries(AUI.Views PRIVATE gtk-3)
		endif ()
	endif()



	target_include_directories(AUI.Views PUBLIC ${GLEW_INCLUDE_DIRS})
	target_include_directories(AUI.Views PRIVATE ${FREETYPE_INCLUDE_DIRS})
	target_compile_definitions(AUI.Views PRIVATE UNICODE=1)

	target_link_libraries(AUI.Views PUBLIC AUI.Core)

	if(ANDROID)
		target_link_libraries(AUI.Views PUBLIC EGL GLESv2 GLESv3)
		target_link_libraries(AUI.Views PRIVATE freetype)
	else()
		target_link_libraries(AUI.Views PRIVATE Freetype::Freetype)
		target_link_libraries(AUI.Views PUBLIC OpenGL::GL)

		if(WIN32)
			target_link_libraries(AUI.Views PUBLIC GLEW::glew_s)
			target_link_libraries(AUI.Views PRIVATE dwmapi)
			target_link_libraries(AUI.Views PRIVATE winmm)
			if(MINGW)
				find_package(ZLIB REQUIRED)
				target_link_libraries(AUI.Views PRIVATE ZLIB::ZLIB)
			endif()
		else()
			target_link_libraries(AUI.Views PUBLIC GLEW::glew)
		endif()
	endif()
else()
	message("OpenGL library was not found. Disabling AUI.Views")
endif()
