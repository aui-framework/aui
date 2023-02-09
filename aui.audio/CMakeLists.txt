cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio WHOLEARCHIVE EXPORT aui)
add_subdirectory(3rdparty/ogg)
aui_link(aui.audio PRIVATE aui::core aui::views)
aui_link(aui.audio PUBLIC ogg)