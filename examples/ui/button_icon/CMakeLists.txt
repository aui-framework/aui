cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.button_icon)
aui_compile_assets(aui.example.button_icon)
aui_link(aui.example.button_icon PRIVATE aui::core aui::views)
