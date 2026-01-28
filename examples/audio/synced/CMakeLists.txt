cmake_minimum_required(VERSION 3.10)

get_filename_component(_t ${CMAKE_CURRENT_SOURCE_DIR} NAME)

aui_executable("aui.example.audio.${_t}")
aui_link("aui.example.audio.${_t}" PRIVATE aui::core aui::views aui::audio)
aui_compile_assets("aui.example.audio.${_t}")
