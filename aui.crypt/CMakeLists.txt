cmake_minimum_required(VERSION 3.10)


unset(OPENSSL_CRYPTO_LIBRARY CACHE)
unset(OPENSSL_SSL_LIBRARY CACHE)
if(AUIB_DISABLE)
    find_package(OpenSSL REQUIRED)
else()
    auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake/archive/56ee19b7e11b418e0f29825414abdca96ff6b83e.zip ARCHIVE)
endif()

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui::core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})

aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)

auib_mark_var_forwardable(OPENSSL_CRYPTO_LIBRARY)
auib_mark_var_forwardable(OPENSSL_SSL_LIBRARY)
auib_mark_var_forwardable(OPENSSL_LIBRARIES)

if(WIN32 AND NOT BUILD_SHARED_LIBS)
    aui_link(OpenSSL::Crypto INTERFACE wsock32 ws2_32)
endif()
