cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.backdrop)
aui_compile_assets(aui.example.backdrop)
aui_link(aui.example.backdrop PRIVATE aui::core aui::views)
