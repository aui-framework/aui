cmake_minimum_required(VERSION 3.10)


unset(OPENSSL_CRYPTO_LIBRARY CACHE)
unset(OPENSSL_SSL_LIBRARY CACHE)
auib_import(OpenSSL https://github.com/aui-framework/openssl-cmake/archive/56ee19b7e11b418e0f29825414abdca96ff6b83e.zip ARCHIVE)

aui_module(aui.crypt EXPORT aui)
aui_enable_tests(aui.crypt)
aui_link(aui.crypt PUBLIC aui::core)
target_include_directories(aui.crypt PRIVATE ${OPENSSL_INCLUDE_DIR})

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    set(VC_LTL_VERSION v5.2.2)
    set(WindowsTargetPlatformMinVersion "5.1.2600.0")
    set(SupportLTL                      "ucrt")
    set(LTLPlatform                     "Win32")

    FetchContent_Declare(vc_ltl 
        URL https://github.com/Chuyu-Team/VC-LTL5/releases/download/${VC_LTL_VERSION}/VC-LTL-Binary.7z
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(vc_ltl)
    include("${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")

    message(STATUS "[AUI] Import: VC-LTL ${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")
    aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto YY_Thunks)
    if(BUILD_SHARED_LIBS)
        set_target_properties(aui.crypt PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
    else()
        set_target_properties(aui.crypt PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:mainCRTStartup")
    endif()
else()
    aui_link(aui.crypt PRIVATE OpenSSL::SSL OpenSSL::Crypto)
endif()

if(WIN32 AND NOT BUILD_SHARED_LIBS)
    aui_link(OpenSSL::Crypto INTERFACE wsock32 ws2_32)
endif()
