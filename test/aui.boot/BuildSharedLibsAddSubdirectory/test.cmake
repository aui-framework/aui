cmake_minimum_required(VERSION 3.10)

# Checks that BUILD_SHARED_LIBS variable is shared with dependency when importing it with ADD_SUBDIRECTORY

if (NOT AUI_TEST_BOOT_BRANCH)
    message(FATAL_ERROR "AUI_TEST_BOOT_BRANCH is not set")
endif()

include(../helper.cmake)
