cmake_minimum_required(VERSION 3.10)

# This test imports aui::core through aui.boot, configures, compiles the project and runs the test program.

include(../helper.cmake)

execute_process(COMMAND ${CMAKE_COMMAND} --build . --target Tests
        WORKING_DIRECTORY b
        RESULT_VARIABLE _r)

if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "Build failed")
endif()

find_program(_test_project Tests PATHS b/bin b/bin/Debug REQUIRED)

execute_process(COMMAND ${_test_project} RESULT_VARIABLE _r)
if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "Test program failed")
endif()
