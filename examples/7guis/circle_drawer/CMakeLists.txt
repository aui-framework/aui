cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.circle_drawer)
aui_link(aui.example.circle_drawer PRIVATE aui::views)
