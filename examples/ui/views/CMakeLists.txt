project(aui.example.views)
cmake_minimum_required(VERSION 3.16)

# Uncomment this code to pull AUI:
#
# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views audio curl)

aui_executable(aui.example.views)
aui_compile_assets(aui.example.views)

# [aui_link]
if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
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
aui_link(aui.example.views PRIVATE aui::core aui::views aui::audio aui::curl YY_Thunks)
set_target_properties(aui.example.views PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
aui_link(aui.example.views PRIVATE aui::core aui::views aui::audio aui::curl)
endif()
# [aui_link]

# [aui_app]
aui_app(TARGET aui.example.views
        NAME "AUI Examples"
        APPLE_BUNDLE_IDENTIFIER "aui.Examples"
        APPLE_SIGN_IDENTITY "iPhone Developer"
        APPLE_TEAM_ID "98S256N2KJ"
        ANDROID_PACKAGE "com.github.aui.examples"
        ICON "assets/img/logo.svg"
        )
# [aui_app]
