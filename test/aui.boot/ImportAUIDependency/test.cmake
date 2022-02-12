cmake_minimum_required(VERSION 3.10)

# This test imports aui::core through aui.boot, configures, compiles the project and runs the test program.

include(../helper.cmake)

find_program(_test_project test_project PATHS b/bin b/bin/Debug REQUIRED)

execute_process(COMMAND ${_test_project} RESULT_VARIABLE _r)
if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "Test program failed")
endif()