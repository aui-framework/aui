cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.timer)
aui_link(aui.example.timer PRIVATE aui::views)
