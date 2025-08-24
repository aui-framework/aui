cmake_minimum_required(VERSION 3.10)

auib_import(ctre https://github.com/hanickadot/compile-time-regular-expressions
            VERSION v3.9.0)

aui_executable(aui.example.flight_booker)
if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../../../WinXPuseLTL5.cmake)
    aui_link(aui.example.flight_booker PRIVATE aui::views ctre::ctre YY_Thunks)
    set_target_properties(aui.example.flight_booker PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
    aui_link(aui.example.flight_booker PRIVATE aui::views ctre::ctre)
endif()
