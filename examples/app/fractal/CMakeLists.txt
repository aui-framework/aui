cmake_minimum_required(VERSION 3.10)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

aui_executable(aui.example.fractal)
aui_compile_assets(aui.example.fractal)

aui_link(aui.example.fractal PRIVATE aui::core aui::views)
