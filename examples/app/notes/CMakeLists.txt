project(aui.example.notes)
cmake_minimum_required(VERSION 3.16)

# Uncomment this code to pull AUI:
#
# # Download aui.boot.cmake (one-time):
# #   curl https://raw.githubusercontent.com/aui-framework/aui/refs/heads/develop/aui.boot.cmake -o aui.boot.cmake
# #
# include(aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views json)

aui_executable(aui.example.notes)
aui_compile_assets(aui.example.notes)

aui_link(aui.example.notes PUBLIC aui::core aui::views aui::json)
