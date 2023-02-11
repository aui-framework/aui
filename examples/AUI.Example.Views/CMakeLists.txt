project(aui.example.views)
cmake_minimum_required(VERSION 3.15)

# In your projects you should use this code in order to pull aui:

# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views)
#
# using local repository instead in order to make checks easier in CI.

aui_executable(aui.example.views)
aui_compile_assets(aui.example.views)

aui_link(aui.example.views PRIVATE aui::core aui::views aui::audio)

aui_app(TARGET aui.example.views
        NAME "AUI Examples"
        APPLE_BUNDLE_IDENTIFIER "aui.Examples"
        APPLE_SIGN_IDENTITY "iPhone Developer"
        APPLE_TEAM_ID "98S256N2KJ"
        ANDROID_PACKAGE "com.github.aui.examples"
        ICON "assets/img/logo.svg"
        )
