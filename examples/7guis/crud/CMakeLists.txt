cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.crud)
aui_link(aui.example.crud PRIVATE aui::views)
