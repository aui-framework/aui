# checks aui.toolbox compilation and invocation in crosscompilation environment (e.g. building for android)

cmake_minimum_required(VERSION 3.16)

project(test)

add_subdirectory(../../ test)
set(AUI_TOOLBOX_EXE "")
_aui_provide_toolbox_for_host()

if (NOT AUI_TOOLBOX_EXE)
    message(FATAL_ERROR "AUI_TOOLBOX_EXE is not set")
endif()

if (NOT EXISTS ${AUI_TOOLBOX_EXE})
    message(FATAL_ERROR "AUI_TOOLBOX_EXE (${AUI_TOOLBOX_EXE}) does not exists")
endif()

# try to invoke AUI_TOOLBOX_EXE
execute_process(COMMAND ${AUI_TOOLBOX_EXE}
        OUTPUT_VARIABLE _o
        RESULT_VARIABLE _r)
if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "aui.toolbox failed")
endif()

if (NOT _o MATCHES ".*Advanced Universal Interface toolbox.*")
    message(FATAL_ERROR "aui.toolbox printed invalid output\nnote: output:\n${_o}")
endif()