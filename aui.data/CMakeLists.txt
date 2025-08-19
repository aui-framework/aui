cmake_minimum_required(VERSION 3.10)

aui_module(aui.data EXPORT aui)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.data PRIVATE aui::core YY_Thunks)
else()
    aui_link(aui.data PRIVATE aui::core)
endif()
