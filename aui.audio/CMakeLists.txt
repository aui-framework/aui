cmake_minimum_required(VERSION 3.10)

aui_module(aui.audio EXPORT aui)

aui_link(aui.audio PRIVATE aui.core)
