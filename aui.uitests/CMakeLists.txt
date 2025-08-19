cmake_minimum_required(VERSION 3.10)

if (NOT TARGET aui::views)
  message(STATUS "aui.views not found - disabling aui.uitests")
  return()
endif()
aui_module(aui.uitests EXPORT aui)

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
  aui_link(aui.uitests PRIVATE aui::core aui::views YY_Thunks)
else()
  aui_link(aui.uitests PRIVATE aui::core aui::views)
endif()

if (TARGET GTest::gmock)
  aui_link(aui.uitests PRIVATE GTest::gmock)
endif()

aui_enable_tests(aui.uitests)
aui_enable_benchmarks(aui.uitests)
