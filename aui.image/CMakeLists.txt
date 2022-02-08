cmake_minimum_required(VERSION 3.10)

aui_module(aui.image EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
set_target_properties(aui.image PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)
aui_link(aui.image PRIVATE aui::core)
