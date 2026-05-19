cmake_minimum_required(VERSION 3.22)

# Test: "cmake -P aui.boot.cmake" with no arguments defaults to help.

include(${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake)

auib_cli_run()

if(NOT _auib_cli_result EQUAL 0)
    message(FATAL_ERROR "[FAIL] no-args exited with code ${_auib_cli_result}, expected 0")
endif()

auib_cli_check_contains("${_auib_cli_stdout}" "${_auib_cli_stderr}" "AUI.Boot")

message(STATUS "[PASS] No arguments defaults to help")
