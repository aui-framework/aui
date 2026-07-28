cmake_minimum_required(VERSION 3.10)

if(AUIB_DISABLE)
    find_package(lunasvg REQUIRED CONFIG)
else()
    auib_import(lunasvg https://github.com/sammycage/lunasvg
                VERSION v3.3.0
                CMAKE_ARGS -DLUNASVG_BUILD_EXAMPLES=OFF -DPLUTOVG_BUILD_EXAMPLES=OFF
    )
endif()

set(WEBP_COMPONENTS_TO_DISABLE ANIM_UTILS CWEBP DWEBP GIF2WEBP IMG2WEBP VWEBP WEBPINFO LIBWEBPMUX WEBPMUX EXTRAS)

foreach(_component ${WEBP_COMPONENTS_TO_DISABLE})
    list(APPEND WEBP_CMAKE_ARGS "-DWEBP_BUILD_${_component}=OFF")
endforeach()

if(AUIB_DISABLE)
    find_package(WebP REQUIRED CONFIG)
else()
    auib_import(WebP https://github.com/webmproject/libwebp VERSION v1.6.0 CMAKE_ARGS ${WEBP_CMAKE_ARGS})
endif()

aui_module(aui.image WHOLEARCHIVE EXPORT aui)

if(NOT MSVC OR (MSVC_VERSION GREATER_EQUAL 1900))
    include(CheckSymbolExists)
    check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
    if(HAVE_SNPRINTF)
        target_compile_definitions(aui.image PUBLIC HAVE_SNPRINTF=1)
    endif()
endif()

add_subdirectory(3rdparty)

aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)

aui_enable_tests(aui.image)
