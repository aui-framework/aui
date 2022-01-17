cmake_minimum_required(VERSION 3.10)

# This test imports aui::core through aui.boot, configures, compiles the project and runs the test program.

if (NOT AUI_TEST_BOOT_BRANCH)
    message(FATAL_ERROR "AUI_TEST_BOOT_BRANCH is not set")
endif()

include(../helper.cmake)

find_program(_test_project test_project PATHS b/bin b/bin/Debug REQUIRED)

execute_process(COMMAND ${_test_project}
                COMMAND_ERROR_IS_FATAL ANY)