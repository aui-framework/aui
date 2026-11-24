cmake_minimum_required(VERSION 3.10)
project(test_project)

include(../../../../aui.boot.cmake)

auib_import(base ${CMAKE_CURRENT_SOURCE_DIR}/../base_${TEST_BASE_VERSION} VERSION ${TEST_BASE_VERSION})
if (TEST_NO_REQUIRES)
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid)
else()
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid REQUIRES base)
endif()

# mid's copy, included by add_subdirectory, must not redefine the project's aui.boot.cmake of the same version
get_property(_mid_boot_in_use GLOBAL PROPERTY MID_BOOT_IN_USE)
if (_mid_boot_in_use)
    message(FATAL_ERROR "mid's aui.boot.cmake has redefined the project's one")
endif()

add_executable(test_project main.cpp)
target_link_libraries(test_project PRIVATE mid::mid)
