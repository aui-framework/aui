cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.contacts)

aui_link(aui.example.contacts PRIVATE aui::core aui::views)
