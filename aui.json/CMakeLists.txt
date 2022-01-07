cmake_minimum_required(VERSION 3.10)

aui_module(aui.json EXPORT aui)

target_link_libraries(aui.json PRIVATE aui.core)