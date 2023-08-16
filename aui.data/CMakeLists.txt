cmake_minimum_required(VERSION 3.10)

aui_module(aui.data EXPORT aui)

aui_link(aui.data PRIVATE aui::core)
