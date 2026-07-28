cmake_minimum_required(VERSION 3.22)

# Test: "cmake -P aui.boot.cmake <unknown>" exits with non-zero and prints error.

include(${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake)

auib_cli_run(this_command_does_not_exist)

if(_auib_cli_result EQUAL 0)
    message(FATAL_ERROR "[FAIL] unknown command exited with code 0, expected non-zero")
endif()

auib_cli_check_contains("${_auib_cli_stdout}" "${_auib_cli_stderr}" "Unknown command")

message(STATUS "[PASS] Unknown command correctly fails")
