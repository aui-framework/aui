# Shared helper for aui.boot CLI mode tests.
#
# Provides:
#   AUI_BOOT_SCRIPT  - absolute path to aui.boot.cmake
#   auib_cli_run()   - runs cmake -P aui.boot.cmake with given args
#   auib_cli_check_contains() - checks text contains keyword in stdout/stderr

cmake_minimum_required(VERSION 3.22)

set(AUI_BOOT_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/../../../aui.boot.cmake")

# Runs cmake -P aui.boot.cmake <args>...
# Sets _auib_cli_result, _auib_cli_stdout, _auib_cli_stderr in parent scope.
function(auib_cli_run)
    execute_process(
        COMMAND ${CMAKE_COMMAND} -P "${AUI_BOOT_SCRIPT}" ${ARGV}
        OUTPUT_VARIABLE _out
        ERROR_VARIABLE _err
        RESULT_VARIABLE _res
    )
    set(_auib_cli_result "${_res}" PARENT_SCOPE)
    set(_auib_cli_stdout "${_out}" PARENT_SCOPE)
    set(_auib_cli_stderr "${_err}" PARENT_SCOPE)
endfunction()

# Fatal error if _keyword is not found in either _output or _error.
function(auib_cli_check_contains _output _error _keyword)
    string(FIND "${_output}" "${_keyword}" _pos_out)
    string(FIND "${_error}" "${_keyword}" _pos_err)
    if(_pos_out EQUAL -1 AND _pos_err EQUAL -1)
        message(FATAL_ERROR "[FAIL] output does not contain '${_keyword}'")
    endif()
endfunction()
