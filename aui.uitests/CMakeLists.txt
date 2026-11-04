cmake_minimum_required(VERSION 3.10)

if (NOT TARGET aui::views)
  message(STATUS "aui.views not found - disabling aui.uitests")
  return()
endif()
aui_module(aui.uitests EXPORT aui)
aui_link(aui.uitests PUBLIC aui::core aui::views)   # public headers include views

if(AUIB_DISABLE)
  find_package(GTest REQUIRED)
endif()

if (TARGET GTest::gtest)
  aui_link(aui.uitests PUBLIC GTest::gtest)   # UITest.h includes gtest
endif()
if (TARGET GTest::gmock)
  aui_link(aui.uitests PUBLIC GTest::gmock)
endif()

aui_enable_tests(aui.uitests)
if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
aui_enable_benchmarks(aui.uitests)
endif()
