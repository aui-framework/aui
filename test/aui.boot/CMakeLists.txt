cmake_minimum_required(VERSION 3.10)

foreach(_varname
        BUILD_SHARED_LIBS
        )
    list(APPEND TEST_CMAKE_ARGS "-D${_varname}=${${_varname}}")
endforeach()

file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/helper.cmake "set(TEST_CMAKE_ARGS \"${TEST_CMAKE_ARGS}\")"
        [[

file(REMOVE_RECURSE b)
file(MAKE_DIRECTORY ${directory} b)
execute_process(COMMAND ${CMAKE_COMMAND} ../test_project ${TEST_CMAKE_ARGS}
                WORKING_DIRECTORY b
                RESULT_VARIABLE _r)
if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "Test program failed")
endif()

execute_process(COMMAND ${CMAKE_COMMAND} --build .
                WORKING_DIRECTORY b
                RESULT_VARIABLE _r)
if (NOT _r STREQUAL "0")
    message(FATAL_ERROR "Test program failed")
endif()
        ]])

function(define_aui_boot_test NAME)
    add_test(NAME "aui.build.boot.${NAME}"
            COMMAND
            ${CMAKE_COMMAND} -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} -DTARGET_GENERATOR=${CMAKE_GENERATOR} -P test.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME})

endfunction()

define_aui_boot_test(BasicProgram)
define_aui_boot_test(ImportAUIDependency)
define_aui_boot_test(Tests)
define_aui_boot_test(BuildSharedLibsAddSubdirectory)

# github ci/cd  could not handle this test for unknown reason
# define_aui_boot_test(Assets)
