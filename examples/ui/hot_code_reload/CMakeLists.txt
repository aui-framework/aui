cmake_minimum_required(VERSION 3.10)

if (NOT AUI_PLATFORM_LINUX)
    return()
endif ()

# [aui_enable_hotswap]
aui_executable(aui.example.hot_code_reload)
aui_link(aui.example.hot_code_reload PRIVATE aui::core aui::views aui::remote_tools)

aui_enable_hotswap(aui.example.hot_code_reload)
# [aui_enable_hotswap]
