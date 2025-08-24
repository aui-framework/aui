cmake_minimum_required(VERSION 3.10)

aui_executable(aui.example.infinite_lazy_list)

if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../../../WinXPuseLTL5.cmake)
    aui_link(aui.example.infinite_lazy_list PRIVATE aui::core aui::views YY_Thunks)
    set_target_properties(aui.example.infinite_lazy_list PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
    aui_link(aui.example.infinite_lazy_list PRIVATE aui::core aui::views)
endif()
