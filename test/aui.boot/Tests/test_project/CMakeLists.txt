cmake_minimum_required(VERSION 3.10)

project(test_project)

set(AUI_CACHE_DIR ${CMAKE_BINARY_DIR} CACHE PATH "")
include(../../../../aui.boot.cmake)
auib_import(aui ../../../../
            COMPONENTS core)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core)
aui_enable_tests()