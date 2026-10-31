cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.remote_debug)
aui_link(aui.example.remote_debug PRIVATE aui::core aui::views)

# [aui_link_dev_extras]
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    # recommended: restrict dev_extras to debug builds only
    aui_link(aui.example.remote_debug PRIVATE aui::dev_extras)
endif ()
# [aui_link_dev_extras]
