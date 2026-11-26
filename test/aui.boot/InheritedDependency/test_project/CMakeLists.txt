cmake_minimum_required(VERSION 3.10)
project(test_project)

include(../../../../aui.boot.cmake)

# how mid gets the project's base: OVERRIDE (default), REQUIRES (deprecated name) or NONE
if (NOT TEST_OVERRIDE_KW)
    set(TEST_OVERRIDE_KW OVERRIDE)
endif()
set(_mid_args)
if (NOT TEST_OVERRIDE_KW STREQUAL "NONE")
    list(APPEND _mid_args ${TEST_OVERRIDE_KW} base)
endif()
if (TEST_MID_PRECOMPILED_URL_PREFIX)
    list(APPEND _mid_args PRECOMPILED_URL_PREFIX ${TEST_MID_PRECOMPILED_URL_PREFIX})
endif()

# a package config that replays an import of base v0, as configs of packages built with AUI.Boot do
if (TEST_REPLAYER)
    find_package(replayer REQUIRED PATHS ${CMAKE_CURRENT_SOURCE_DIR}/../replayer NO_DEFAULT_PATH)
endif()

if (TEST_MID_FIRST)
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid ${_mid_args})
endif()
auib_import(base ${CMAKE_CURRENT_SOURCE_DIR}/../base_${TEST_BASE_VERSION} VERSION ${TEST_BASE_VERSION})
if (NOT TEST_MID_FIRST)
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid ${_mid_args})
endif()

if (TEST_REIMPORT_VERSION)
    auib_import(base ${CMAKE_CURRENT_SOURCE_DIR}/../base_${TEST_REIMPORT_VERSION} VERSION ${TEST_REIMPORT_VERSION})
endif()

# mid's copy, included by add_subdirectory, must not redefine the project's aui.boot.cmake of the same version
get_property(_mid_boot_in_use GLOBAL PROPERTY MID_BOOT_IN_USE)
if (_mid_boot_in_use)
    message(FATAL_ERROR "mid's aui.boot.cmake has redefined the project's one")
endif()

add_executable(test_project main.cpp)
target_link_libraries(test_project PRIVATE mid::mid)
