cmake_minimum_required(VERSION 3.10)

aui_module(aui.json EXPORT aui)

aui_link(aui.json PRIVATE aui.core)