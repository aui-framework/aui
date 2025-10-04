cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.hot_code_reload)
aui_link(aui.example.hot_code_reload PRIVATE aui::core aui::views)
target_compile_options(aui.example.hot_code_reload PRIVATE -falign-functions=1024)
