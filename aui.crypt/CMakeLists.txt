cmake_minimum_required(VERSION 3.10)

if(AUIB_DISABLE)
    find_package(MbedTLS REQUIRED)
else()
    auib_import(MbedTLS https://github.com/aui-framework/mbedtls
                VERSION d0c567d79ef37d672a3c7c49a2dc90754e246ffd
                CMAKE_ARGS -DENABLE_PROGRAMS=OFF -DENABLE_TESTING=OFF
    )
endif()

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui::core)
aui_link(aui.crypt PRIVATE MbedTLS::mbedcrypto)

set(MBEDTLS_INCLUDE_DIR "${MBEDTLS_ROOT_DIR}/include" CACHE STRING "MbedTLS include directory work-around")

auib_mark_var_forwardable(MBEDTLS_VERSION)
auib_mark_var_forwardable(MBEDTLS_LIBRARIES)
auib_mark_var_forwardable(MBEDTLS_INCLUDE_DIR)
auib_mark_var_forwardable(MBEDTLS_ROOT_DIR)
