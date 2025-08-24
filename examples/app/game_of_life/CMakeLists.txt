project(aui.example.game_of_life)
cmake_minimum_required(VERSION 3.16)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

# Uncomment this code to pull AUI:
#
# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views json)

aui_executable(aui.example.game_of_life)
if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../../../WinXPuseLTL5.cmake)
    aui_link(aui.example.game_of_life PRIVATE aui::views YY_Thunks)
    set_target_properties(aui.example.game_of_life PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
    aui_link(aui.example.game_of_life PRIVATE aui::views)
endif()

