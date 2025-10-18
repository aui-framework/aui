cmake_minimum_required(VERSION 3.10)

option(AUI_SHOW_TOUCHES "Show debug touches" OFF)

cmake_policy(SET CMP0072 NEW)

auib_use_system_libs_begin()
find_package(OpenGL)
auib_use_system_libs_end()

if (OPENGL_FOUND OR ANDROID OR IOS)
    if(AUIB_DISABLE)
        find_package(freetype REQUIRED CONFIG)
    else()
        auib_import(Freetype https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.zip ARCHIVE
                    CONFIG_ONLY
                    CMAKE_ARGS -DFT_DISABLE_BZIP2=ON -DFT_DISABLE_PNG=ON -DFT_DISABLE_HARFBUZZ=ON -DFT_DISABLE_BROTLI=ON
        )
    endif()

    if (NOT TARGET freetype)
        message(FATAL_ERROR "Freetype not imported")
    endif ()


    aui_module(aui.views EXPORT aui)
    aui_enable_tests(aui.views)

    if (AUI_PLATFORM_LINUX)
        auib_use_system_libs_begin()
        find_package(PkgConfig REQUIRED)

        pkg_check_modules(DBUS REQUIRED IMPORTED_TARGET dbus-1)
        pkg_check_modules(GLIB REQUIRED IMPORTED_TARGET glib-2.0)
        pkg_check_modules(GIO REQUIRED IMPORTED_TARGET gio-2.0)

        auib_use_system_libs_end()

        aui_link(aui.views PRIVATE
                X11::X11 X11::Xrandr X11::Xcursor X11::Xi
                Fontconfig::Fontconfig
                PkgConfig::DBUS
                PkgConfig::GLIB
                PkgConfig::GIO
        )

        file(GLOB_RECURSE sigs src/ *.sigs)
        _aui_check_toolbox()
        foreach (sig ${sigs})
            get_filename_component(_out ${sig} NAME)
            set(_out "${CMAKE_CURRENT_BINARY_DIR}/${_out}.cpp")
            add_custom_command(
                    DEPENDS ${sig}
                    OUTPUT ${_out}
                    COMMAND ${AUI_TOOLBOX_EXE}
                    ARGS generate_dl_stubs ${sig} ${_out}
            )
            target_sources(aui.views PRIVATE ${_out})
        endforeach ()
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

    target_compile_definitions(aui.views PRIVATE
        $<$<OR:$<PLATFORM_ID:MinGW>,$<PLATFORM_ID:Windows>>:UNICODE=1>
        $<$<CXX_COMPILER_ID:AppleClang>:GL_SILENCE_DEPRECATION=1>
    )
    if (AUI_SHOW_TOUCHES)
        target_compile_definitions(aui.views PUBLIC AUI_SHOW_TOUCHES=1)
    endif ()


    aui_link(aui.views PUBLIC aui::core aui::image)
    aui_link(aui.views PRIVATE aui::xml)

    if(${CMAKE_VERSION} VERSION_GREATER_EQUAL "3.19.0")
        set_target_properties(freetype PROPERTIES INSTALL_NAME_DIR "@rpath" BUILD_WITH_INSTALL_RPATH 1)
    endif ()
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
            auib_use_system_libs_begin()
            find_package(X11 REQUIRED)
            find_package(Fontconfig REQUIRED)
            auib_use_system_libs_end()
            aui_link(aui.views PRIVATE epoxy)
        endif ()

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