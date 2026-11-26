cmake_minimum_required(VERSION 3.10)
project(comp_project)

include(../../../../aui.boot.cmake)

auib_import(comp ${CMAKE_CURRENT_SOURCE_DIR}/../comp COMPONENTS ${TEST_COMPONENTS})
