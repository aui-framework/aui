cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.temperature_converter)
aui_link(aui.example.temperature_converter PRIVATE aui::views)
