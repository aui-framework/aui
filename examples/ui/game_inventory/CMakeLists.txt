cmake_minimum_required(VERSION 3.10)

get_filename_component(_t ${CMAKE_CURRENT_SOURCE_DIR} NAME)

aui_executable("aui.example.${_t}")
aui_compile_assets("aui.example.${_t}")
aui_link("aui.example.${_t}" PRIVATE aui::core aui::views)
