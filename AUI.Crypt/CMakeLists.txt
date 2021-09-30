cmake_minimum_required(VERSION 3.10)

auib_import(OpenSSL https://github.com/Alex2772/openssl-cmake)

aui_module(AUI.Crypt)
target_link_libraries(AUI.Crypt PRIVATE AUI.Core)
target_include_directories(AUI.Crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(AUI.Crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)