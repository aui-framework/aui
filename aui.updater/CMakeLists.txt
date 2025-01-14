cmake_minimum_required(VERSION 3.10)

aui_module(aui.updater EXPORT aui)
aui_link(aui.updater PRIVATE aui::core)
aui_enable_tests(aui.updater)
