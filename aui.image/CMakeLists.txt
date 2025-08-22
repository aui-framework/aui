cmake_minimum_required(VERSION 3.10)

auib_import(lunasvg https://github.com/sammycage/lunasvg
            VERSION v3.3.0
            CMAKE_ARGS -DLUNASVG_BUILD_EXAMPLES=OFF -DPLUTOVG_BUILD_EXAMPLES=OFF
)


set(WEBP_COMPONENTS_TO_DISABLE ANIM_UTILS CWEBP DWEBP GIF2WEBP IMG2WEBP VWEBP WEBPINFO LIBWEBPMUX WEBPMUX EXTRAS)

foreach(_component ${WEBP_COMPONENTS_TO_DISABLE})
    list(APPEND WEBP_CMAKE_ARGS "-DWEBP_BUILD_${_component}=OFF")
endforeach()

auib_import(WebP https://github.com/webmproject/libwebp VERSION v1.3.1 CMAKE_ARGS ${WEBP_CMAKE_ARGS})

aui_module(aui.image WHOLEARCHIVE EXPORT aui)

if(NOT MSVC OR (MSVC_VERSION GREATER_EQUAL 1900))
    include(CheckSymbolExists)
    check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
    if(HAVE_SNPRINTF)
        target_compile_definitions(aui.image PUBLIC HAVE_SNPRINTF=1)
    endif()
endif()

add_subdirectory(3rdparty)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(FetchContent)
    set(VC_LTL_VERSION v5.2.2)
    set(WindowsTargetPlatformMinVersion "5.1.2600.0")
    set(SupportLTL                      "ucrt")
    set(LTLPlatform                     "Win32")

    FetchContent_Declare(vc_ltl 
        URL https://github.com/Chuyu-Team/VC-LTL5/releases/download/${VC_LTL_VERSION}/VC-LTL-Binary.7z
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(vc_ltl)

    include("${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")

    aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux YY_Thunks)
    if(BUILD_SHARED_LIBS)
        set_target_properties(aui.image PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
    else()
        set_target_properties(aui.image PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
    endif()
else()
    aui_link(aui.image PRIVATE aui::core lunasvg::lunasvg WebP::webp WebP::webpdemux)
endif()

aui_enable_tests(aui.image)
