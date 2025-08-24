cmake_minimum_required(VERSION 3.10)


unset(OPENSSL_CRYPTO_LIBRARY CACHE)
unset(OPENSSL_SSL_LIBRARY CACHE)
auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake/archive/56ee19b7e11b418e0f29825414abdca96ff6b83e.zip ARCHIVE)

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui::core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../WinXPuseLTL5.cmake)
    aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto YY_Thunks)
    if(BUILD_SHARED_LIBS)
        set_target_properties(aui.crypt PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
    else()
        set_target_properties(aui.crypt PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
    endif()
else()
    aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(WIN32 AND NOT BUILD_SHARED_LIBS)
    aui_link(OpenSSL::Crypto INTERFACE wsock32 ws2_32)
endif()
