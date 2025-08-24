cmake_minimum_required(VERSION 3.10)

aui_module(aui.json EXPORT aui)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(../WinXPuseLTL5.cmake)
    aui_link(aui.json PRIVATE aui::core YY_Thunks)
    if(BUILD_SHARED_LIBS)
        set_target_properties(aui.json PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
    else()
        set_target_properties(aui.json PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
    endif()
else()
    aui_link(aui.json PRIVATE aui::core)
endif()

aui_enable_tests(aui.json)
if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
aui_enable_benchmarks(aui.json)
if (TARGET Benchmarks)
    # curl needed to download the benchmark data
    aui_link(Benchmarks PRIVATE aui::curl)
endif()
endif()