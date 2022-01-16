cmake_minimum_required(VERSION 3.10)

if (NOT TARGET aui.views)
  return()
endif()
aui_module(aui.uitests EXPORT aui)
aui_link(aui.uitests PRIVATE aui.core aui.views GTest::gtest)
