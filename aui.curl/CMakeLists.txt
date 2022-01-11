cmake_minimum_required(VERSION 3.10)

if (MINGW)
    message(STATUS "curl is not supported on MinGW (see https://github.com/aui-framework/aui/issues/31). Disabling aui.curl")
else()
    auib_import(CURL https://github.com/curl/curl
            VERSION curl-7_79_1
            CMAKE_ARGS -DCMAKE_USE_OPENSSL=ON)

    aui_module(aui.curl EXPORT aui)
    aui_link(aui.curl PUBLIC aui.core)
    aui_link(aui.curl PRIVATE CURL::libcurl)
endif()