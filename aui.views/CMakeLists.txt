cmake_minimum_required(VERSION 3.10)

option(AUI_SHOW_TOUCHES "Show debug touches" OFF)

auib_use_system_libs_begin()
cmake_policy(SET CMP0072 NEW)
find_package(OpenGL)
if (AUI_PLATFORM_LINUX)
    find_package(X11 REQUIRED)
    find_package(Fontconfig REQUIRED)
endif ()
auib_use_system_libs_end()

if (OPENGL_FOUND OR ANDROID OR IOS)

#    auib_import(Freetype https://github.com/aui-framework/freetype
    auib_import(Freetype https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.zip ARCHIVE
            CONFIG_ONLY)

    if (NOT TARGET freetype)
        message(FATAL_ERROR "Freetype not imported")
    endif ()


    if (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS)
        auib_import(GLEW https://github.com/aui-framework/glew-cmake
                CMAKE_WORKING_DIR "build/cmake"
                VERSION a80a1dc)
    endif ()

    aui_module(aui.views EXPORT aui)
    aui_enable_tests(aui.views)

    if (AUI_PLATFORM_LINUX)
        find_package(PkgConfig REQUIRED)

        pkg_check_modules(DBUS REQUIRED dbus-1)
        include_directories(${DBUS_INCLUDE_DIRS})
        link_directories(${DBUS_LIBRARY_DIRS})

        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        include_directories(${GTK3_INCLUDE_DIRS})
        link_directories(${GTK3_LIBRARY_DIRS})

        aui_link(aui.views PRIVATE X11::X11 X11::Xrandr X11::Xcursor X11::Xi ${DBUS_LIBRARIES} ${GTK3_LIBRARIES} Fontconfig::Fontconfig)
    endif ()

    if (NOT AUI_PLATFORM_WINDOWS)
        aui_compile_assets(aui.views)
    else ()
        aui_compile_assets(aui.views EXCLUDE uni/font/Roboto.ttf)
    endif ()

    auisl_shader(aui.views basic.vsh)
    auisl_shader(aui.views basic_uv.vsh)
    auisl_shader(aui.views shadow.fsh)
    auisl_shader(aui.views shadow_inner.fsh)
    auisl_shader(aui.views rect_solid.fsh)
    auisl_shader(aui.views rect_solid_rounded.fsh)
    auisl_shader(aui.views rect_textured.fsh)
    auisl_shader(aui.views rect_unblend.fsh)
    auisl_shader(aui.views rect_gradient.fsh)
    auisl_shader(aui.views rect_gradient_rounded.fsh)
    auisl_shader(aui.views border_rounded.fsh)
    auisl_shader(aui.views symbol.vsh)
    auisl_shader(aui.views symbol.fsh)
    auisl_shader(aui.views symbol_sub.fsh)
    auisl_shader(aui.views square_sector.fsh)
    auisl_shader(aui.views line_solid_dashed.fsh)

    target_compile_definitions(aui.views PRIVATE UNICODE=1)
    if (AUI_SHOW_TOUCHES)
        target_compile_definitions(aui.views PUBLIC AUI_SHOW_TOUCHES=1)
    endif ()


    aui_link(aui.views PUBLIC aui::core aui::image)
    aui_link(aui.views PRIVATE aui::xml)
    set_target_properties(freetype PROPERTIES INSTALL_NAME_DIR "@rpath" BUILD_WITH_INSTALL_RPATH 1)
    if (AUI_PLATFORM_ANDROID)
        aui_link(aui.views PRIVATE EGL GLESv2 GLESv3)
        aui_link(aui.views PRIVATE freetype)
    elseif (AUI_PLATFORM_IOS)
        auib_use_system_libs_begin()
        find_library(OPENGL OpenGLES REQUIRED)
        auib_use_system_libs_end()
        aui_link(aui.views PRIVATE ${OPENGL})
        aui_link(aui.views PUBLIC freetype)
    elseif (AUI_PLATFORM_EMSCRIPTEN)
        aui_link(aui.views PUBLIC freetype)
    else ()
        aui_link(aui.views PRIVATE freetype)
        aui_link(aui.views PRIVATE OpenGL::GL)

        if (AUI_PLATFORM_LINUX)
            # glew defines unwanted dependencies to OpenGL which may vary from distro to distro; silence them
            set_target_properties(GLEW::GLEW PROPERTIES INTERFACE_LINK_LIBRARIES "")
        endif ()
        aui_link(aui.views PUBLIC GLEW::GLEW)

        if (WIN32)
            aui_link(aui.views PRIVATE dwmapi)
            aui_link(aui.views PRIVATE winmm)
            aui_link(aui.views PRIVATE shlwapi.lib)
            if (MINGW)
                find_package(ZLIB REQUIRED)
                aui_link(aui.views PRIVATE ZLIB::ZLIB)
            endif ()
        elseif (AUI_PLATFORM_MACOS)
            aui_link(aui.views PRIVATE OpenGL::GL)
            target_link_libraries(aui.views PUBLIC "-framework AppKit"
                    "-framework Cocoa"
                    "-framework CoreData"
                    "-framework Foundation"
                    "-framework QuartzCore")
            if(CMAKE_SYSTEM_VERSION VERSION_GREATER_EQUAL "20.0")
                target_link_libraries(aui.views PUBLIC "-framework UniformTypeIdentifiers")
            endif()
        endif ()
    endif ()
else ()
    message("OpenGL library was not found. Disabling aui.views")
endif ()
