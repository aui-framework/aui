cmake_minimum_required(VERSION 3.22)

# Test: "cmake -P aui.boot.cmake help" prints help and exits with code 0.

include(${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake)

auib_cli_run(help)

if(NOT _auib_cli_result EQUAL 0)
    message(FATAL_ERROR "[FAIL] 'help' command exited with code ${_auib_cli_result}, expected 0")
endif()

foreach(_keyword "AUI.Boot" "Usage:" "help" "update" "create")
    auib_cli_check_contains("${_auib_cli_stdout}" "${_auib_cli_stderr}" "${_keyword}")
endforeach()

message(STATUS "[PASS] 'help' command works correctly")
