cmake_minimum_required(VERSION 3.10)

auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake)

aui_module(aui.crypt)
target_link_libraries(aui.crypt PUBLIC aui.core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)