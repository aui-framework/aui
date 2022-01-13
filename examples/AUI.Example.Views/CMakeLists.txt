cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.views)
aui_compile_assets(aui.example.views)


aui_link(aui.example.views PRIVATE aui.core)
aui_link(aui.example.views PRIVATE aui.views)
