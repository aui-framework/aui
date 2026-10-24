cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.customwindow_titlebar_blur)
aui_compile_assets(aui.example.customwindow_titlebar_blur)
aui_link(aui.example.customwindow_titlebar_blur PRIVATE aui::core aui::views)
