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

aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt)
