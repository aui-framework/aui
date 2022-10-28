cmake_minimum_required(VERSION 3.10)

auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake/archive/refs/heads/master.zip
            ARCHIVE
            PRECOMPILED_URL_PREFIX https://github.com/aui-framework/openssl-cmake/releases/download/refs%2Fheads%2Fmaster)

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui.core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
if(WIN32 AND NOT BUILD_SHARED_LIBS)
    aui_link(OpenSSL::Crypto INTERFACE wsock32 ws2_32)
endif()
