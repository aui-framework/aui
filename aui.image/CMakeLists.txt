cmake_minimum_required(VERSION 3.10)

aui_module(aui.image WHOLEARCHIVE EXPORT aui)
target_include_directories(aui.image PRIVATE 3rdparty)
aui_link(aui.image PRIVATE aui::core)
aui_enable_tests(aui.image)
