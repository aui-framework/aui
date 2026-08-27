cmake_minimum_required(VERSION 3.10)

if(AUIB_DISABLE)
    find_package(CURL REQUIRED)
else()
    if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
        auib_import(CURL https://github.com/curl/curl
                    VERSION curl-7_84_0
                    CMAKE_ARGS -DCURL_USE_MBEDTLS=ON -DCURL_TARGET_WINDOWS_VERSION=0x0501)
    else()
        # AUI_MBEDTLS_ABI is not read by curl -- it only feeds AUI.Boot's build specifier, so that a
        # cached curl is rebuilt whenever mbedtls_config.h changes. curl keeps its own static
        # mbedtls_entropy_context, and enabling MBEDTLS_THREADING_C changed that struct's layout;
        # without this, a stale cached curl links against the new mbedtls and crashes on startup in
        # Curl_mbedtlsthreadlock_unlock_function. Bump the value on every mbedtls config change.
        auib_import(CURL https://github.com/aui-framework/curl/
                    VERSION 75a1d9954a4184b42a9365c7d4225b802530634c
                    CMAKE_ARGS -DCURL_USE_MBEDTLS=ON -DBUILD_CURL_EXE=OFF -DUSE_NGHTTP2=OFF -DCURL_DISABLE_LDAP=ON -DCURL_DISABLE_LDAPS=ON -DUSE_LIBIDN2=OFF -DCURL_USE_LIBPSL=OFF -DCURL_BROTLI=OFF -DCURL_ZSTD=OFF -DCURL_USE_LIBSSH2=OFF -DAUI_MBEDTLS_ABI=threading-1
        )
    endif()
endif()

aui_module(aui.curl EXPORT aui WHOLEARCHIVE)
aui_enable_tests(aui.curl)
aui_link(aui.curl PUBLIC aui::core aui::json)

aui_link(aui.curl PRIVATE CURL::libcurl aui::crypt)
