cmake_minimum_required(VERSION 3.10)

project(test_project)
set(AUI_CACHE_DIR ${CMAKE_BINARY_DIR} CACHE PATH "")

set(BUILD_SHARED_LIBS OFF) # we will check for this value in ../test_library/CMakeLists.txt

include(../../../../aui.boot.cmake)
auib_import(test_library ../test_library)
