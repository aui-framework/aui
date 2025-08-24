project(aui.example.notes)
cmake_minimum_required(VERSION 3.16)

# Uncomment this code to pull AUI:
#
# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views json)

aui_executable(aui.example.notes)
aui_compile_assets(aui.example.notes)

aui_link(aui.example.notes PUBLIC aui::core aui::views aui::json)
