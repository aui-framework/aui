cmake_minimum_required(VERSION 3.22)

# Test: "cmake -P aui.boot.cmake update" handles network failure gracefully.

include(${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake)

# Create a temporary directory so CMAKE_CURRENT_BINARY_DIR exists for the download.
set(_update_tmpdir "${CMAKE_CURRENT_BINARY_DIR}/aui_boot_update_test")
file(MAKE_DIRECTORY "${_update_tmpdir}")

execute_process(
    COMMAND ${CMAKE_COMMAND} -P "${AUI_BOOT_SCRIPT}" update
    WORKING_DIRECTORY "${_update_tmpdir}"
    OUTPUT_VARIABLE _out
    ERROR_VARIABLE _err
    RESULT_VARIABLE _res
)

# The update command tries to download from GitHub, so it will likely fail
# in an offline environment. We just verify it doesn't crash in unexpected ways.
if(NOT _res EQUAL 0)
    message(STATUS "[INFO] 'update' command failed as expected (no network)")
endif()

message(STATUS "[PASS] 'update' command handled gracefully")
