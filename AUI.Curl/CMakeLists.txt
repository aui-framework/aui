cmake_minimum_required(VERSION 3.10)

set(AUI_EMBEDDED_CURL ${AUI_IS_WINDOWS} CACHE BOOL "Use embedded cURL")

find_package(CURL)
AUI_Module(AUI.Curl)
target_link_libraries(AUI.Curl PUBLIC AUI.Core)

if (CURL_FOUND AND NOT AUI_EMBEDDED_CURL)
    target_link_libraries(AUI.Curl PRIVATE CURL::libcurl)
else()
    message(STATUS "Using embedded cURL")

    # if ssl and crypto is compiled as targets, we should force cURL to use it
    if (TARGET ssl)
        set(CMAKE_USE_OPENSSL ON)

        add_library(OpenSSL::SSL INTERFACE IMPORTED)
        add_library(OpenSSL::Crypto INTERFACE IMPORTED)

        # TODO: make embedded libaries to the same in more convenient way
        get_filename_component(OPENSSL_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../AUI.Crypt/3rdparty/openssl-cmake/include REALPATH)
        target_include_directories(OpenSSL::SSL INTERFACE ${OPENSSL_INCLUDE_DIR})
        target_link_libraries(OpenSSL::SSL INTERFACE ssl)
        target_link_libraries(OpenSSL::Crypto INTERFACE crypto)
        set(OPENSSL_CRYPTO_LIBRARY crypto)
        set(OPENSSL_SSL_LIBRARY ssl)
        set(OPENSSL_LIBRARIES ssl crypto)
        #message("OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIR}")
        #message("OPENSSL_LIBRARIES ${OPENSSL_LIBRARIES}")
    endif()

    add_subdirectory(3rdparty/curl)
    target_link_libraries(AUI.Curl PRIVATE libcurl)
endif()