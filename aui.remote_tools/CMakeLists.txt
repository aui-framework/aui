cmake_minimum_required(VERSION 3.10)

aui_module(aui.remote_tools EXPORT aui)

aui_link(aui.remote_tools PRIVATE aui::core)
