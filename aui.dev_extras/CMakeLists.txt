cmake_minimum_required(VERSION 3.10)

aui_module(aui.dev_extras EXPORT aui)

aui_link(aui.dev_extras PRIVATE aui::core)
