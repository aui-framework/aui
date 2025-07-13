cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.button)
aui_link(aui.example.button PRIVATE aui::core aui::views)
