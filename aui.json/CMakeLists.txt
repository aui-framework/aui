cmake_minimum_required(VERSION 3.10)

aui_module(aui.json EXPORT aui)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    aui_link(aui.json PRIVATE aui::core YY_Thunks)
else()
    aui_link(aui.json PRIVATE aui::core)
endif()

aui_enable_tests(aui.json)
aui_enable_benchmarks(aui.json)
if (TARGET Benchmarks)
    # curl needed to download the benchmark data
    aui_link(Benchmarks PRIVATE aui::curl)
endif()