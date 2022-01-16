cmake_minimum_required(VERSION 3.10)

add_test(NAME aui.boot.basic
        COMMAND
        ${CMAKE_COMMAND} -DAUI_TEST_BOOT_BRANCH=${AUI_TEST_BOOT_BRANCH} -P test.cmake
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/basic)