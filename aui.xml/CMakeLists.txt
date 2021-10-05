cmake_minimum_required(VERSION 3.10)

aui_module(aui.xml)

target_link_libraries(aui.xml PRIVATE aui.core)
