cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.counter)
aui_link(aui.example.counter PRIVATE aui::views)
