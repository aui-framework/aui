cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.infinite_lazy_list)

aui_link(aui.example.infinite_lazy_list PRIVATE aui::core aui::views)
