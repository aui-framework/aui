cmake_minimum_required(VERSION 3.10)


set(CMAKE_FIND_DEBUG_MODE TRUE)
auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake/archive/refs/heads/master.zip
            ARCHIVE
            PRECOMPILED_URL_PREFIX https://github.com/aui-framework/openssl-cmake/releases/download/refs%2Fheads%2Fmaster)

# https://github.com/aui-framework/aui/issues/84
get_target_property(_l OpenSSL::Crypto IMPORTED_LOCATION)
if (_l)
    if (NOT _l MATCHES "^${AUI_CACHE_DIR}")
        message(FATAL_ERROR "Internal AUI error: OpenSSL links to the system library (${_l}).")
    endif()
endif()

get_target_property(_l OpenSSL::Crypto IMPORTED_LOCATION_DEBUG)
if (_l AND NOT AUI_IGNORE_OPENSSL)
    if(NOT LIB_EAY_DEBUG MATCHES "^${AUI_CACHE_DIR}")
        message(FATAL_ERROR "CMake linked to manually installed OpenSSL at ${LIB_EAY_DEBUG}. This library should be removed otherwise runtime linking may be broken. Set AUI_IGNORE_OPENSSL=TRUE to ignore.")
    endif()
endif()

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui.core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
if(WIN32 AND NOT BUILD_SHARED_LIBS)
    aui_link(OpenSSL::Crypto INTERFACE wsock32 ws2_32)
endif()
