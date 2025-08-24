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
    include(../../../WinXPuseLTL5.cmake)
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
