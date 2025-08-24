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

if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../../../WinXPuseLTL5.cmake)
    aui_link(aui.example.notes PUBLIC aui::core aui::views aui::json YY_Thunks)
    set_target_properties(aui.example.notes PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
    aui_link(aui.example.notes PUBLIC aui::core aui::views aui::json)
endif()
