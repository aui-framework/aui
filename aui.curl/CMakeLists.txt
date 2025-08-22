cmake_minimum_required(VERSION 3.10)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    auib_import(CURL https://github.com/curl/curl
                VERSION curl-7_84_0
                CMAKE_ARGS -DCURL_TARGET_WINDOWS_VERSION=0x0501)
else()
    auib_import(CURL https://github.com/aui-framework/curl/archive/b8c004e47705bb45c8e3408835302f5c44ce60c7.zip
                ARCHIVE)
endif()
aui_module(aui.curl EXPORT aui WHOLEARCHIVE)
aui_enable_tests(aui.curl)
aui_link(aui.curl PUBLIC aui::core aui::json)

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

    aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt YY_Thunks)
    if(BUILD_SHARED_LIBS)
        set_target_properties(aui.curl PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
    else()
        set_target_properties(aui.curl PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
    endif()
else()
    aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt)
endif()
