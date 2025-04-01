cmake_minimum_required(VERSION 3.10)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

aui_executable(aui.example.minesweeper)
aui_compile_assets(aui.example.minesweeper)

aui_link(aui.example.minesweeper PRIVATE aui::core aui::views)
