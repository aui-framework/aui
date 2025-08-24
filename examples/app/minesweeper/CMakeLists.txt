cmake_minimum_required(VERSION 3.10)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

aui_executable(aui.example.minesweeper)
aui_compile_assets(aui.example.minesweeper)

if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../../../WinXPuseLTL5.cmake)
    aui_link(aui.example.minesweeper PRIVATE aui::core aui::views YY_Thunks)
    set_target_properties(aui.example.minesweeper PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
    aui_link(aui.example.minesweeper PRIVATE aui::core aui::views)
endif()
