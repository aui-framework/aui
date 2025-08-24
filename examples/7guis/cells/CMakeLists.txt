cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.cells)
aui_link(aui.example.cells PRIVATE aui::views)
aui_enable_tests(aui.example.cells)
