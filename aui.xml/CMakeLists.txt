cmake_minimum_required(VERSION 3.10)

aui_module(aui.xml EXPORT aui)

aui_link(aui.xml PRIVATE aui::core)
