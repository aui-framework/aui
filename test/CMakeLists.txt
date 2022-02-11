# cmake build test cases

cmake_minimum_required(VERSION 3.10)

project(test)

enable_testing()

# test aui.boot on ci/cd
add_subdirectory(aui.boot)

macro(define_test_case NAME)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/b)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/b)
    add_test(NAME "aui.build.${NAME}"
            COMMAND
            ${CMAKE_COMMAND} ..
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME}/b)
endmacro()

define_test_case(ProvideToolbox)