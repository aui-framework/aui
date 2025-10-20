cmake_minimum_required(VERSION 3.10)

if(AUIB_DISABLE)
    find_package(CURL REQUIRED)
else()
    if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
        auib_import(CURL https://github.com/curl/curl
                    VERSION curl-7_84_0
                    CMAKE_ARGS -DCURL_USE_MBEDTLS=ON -DCURL_TARGET_WINDOWS_VERSION=0x0501)
    else()
        auib_import(CURL https://github.com/aui-framework/curl/archive/75a1d9954a4184b42a9365c7d4225b802530634c.zip
                    ARCHIVE
                    CMAKE_ARGS -DCURL_USE_MBEDTLS=ON -DBUILD_CURL_EXE=OFF -DUSE_NGHTTP2=OFF -DCURL_DISABLE_LDAP=ON -DCURL_DISABLE_LDAPS=ON -DUSE_LIBIDN2=OFF -DCURL_USE_LIBPSL=OFF -DCURL_BROTLI=OFF -DCURL_ZSTD=OFF -DCURL_USE_LIBSSH2=OFF
        )
    endif()
endif()

if(AUIB_DISABLE)
    find_package(ixwebsocket REQUIRED)
else ()
    auib_import(ixwebsocket https://github.com/machinezone/IXWebSocket
                VERSION v11.4.6
                CMAKE_ARGS -DUSE_TLS=ON -DUSE_OPEN_SSL=OFF
    )
endif ()

aui_module(aui.curl EXPORT aui WHOLEARCHIVE)
aui_enable_tests(aui.curl)
aui_link(aui.curl PUBLIC aui::core aui::json)

aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt ixwebsocket::ixwebsocket)
