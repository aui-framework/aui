cmake_minimum_required(VERSION 3.10)


if (TARGET aui.views)
aui_module(aui.svg)

target_include_directories(aui.svg PRIVATE 3rdparty/nanosvg/src)

target_link_libraries(aui.svg PRIVATE aui.core)
target_link_libraries(aui.svg PRIVATE aui.views)
target_link_libraries(aui.svg PRIVATE aui.xml)
else()
    message("Could not find aui.views; Disabling aui.svg")
endif()
