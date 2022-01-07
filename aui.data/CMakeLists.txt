cmake_minimum_required(VERSION 3.10)

aui_module(aui.data EXPORT aui)

target_link_libraries(aui.data PRIVATE aui.core)
