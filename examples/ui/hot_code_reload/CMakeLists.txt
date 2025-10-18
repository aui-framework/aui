cmake_minimum_required(VERSION 3.10)

if (NOT AUI_PLATFORM_LINUX)
    return()
endif ()

# [aui_enable_hotswap]
aui_executable(aui.example.hot_code_reload)
aui_link(aui.example.hot_code_reload PRIVATE aui::core aui::views)

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # recommended: restrict remote_tools to debug builds only
    aui_link(aui.example.hot_code_reload PRIVATE aui::remote_tools)
    aui_enable_hotswap(aui.example.hot_code_reload)
endif()
# [aui_enable_hotswap]
