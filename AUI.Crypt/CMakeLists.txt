cmake_minimum_required(VERSION 3.10)

set(AUI_EMBEDDED_OPENSSL ${AUI_IS_WINDOWS} CACHE BOOL "Use embedded OpenSSL")

auib_import(OpenSSL https://github.com/Alex2772/openssl-cmake)

aui_module(AUI.Crypt)
target_link_libraries(AUI.Crypt PRIVATE AUI.Core)
target_include_directories(AUI.Crypt PRIVATE ${OPENSSL_INCLUDE_DIR})
target_link_libraries(AUI.Crypt PRIVATE ssl crypto)