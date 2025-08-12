cmake_minimum_required(VERSION 3.10)


list(APPEND TEST_CMAKE_ARGS "-G${CMAKE_GENERATOR}")
list(APPEND TEST_CMAKE_ARGS "-DAUIB_TRACE_BUILD_SYSTEM=ON")

foreach(_varname
        BUILD_SHARED_LIBS AUI_TEST_DEPLOY_VERSION AUI_TEST_DEPLOY_REPO
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
            ${CMAKE_COMMAND} -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} -P test.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME})

endfunction()

if (DEFINED AUI_TEST_DEPLOY_VERSION)
    message(STATUS "AUI_TEST_DEPLOY_VERSION=${AUI_TEST_DEPLOY_VERSION} (deployment tests only)")
    define_aui_boot_test(Precompiled1)
    define_aui_boot_test(Precompiled2)
    if(AUI_PLATFORM_LINUX)
        # this test has hardcoded path to linux binary
        define_aui_boot_test(Precompiled3)
    endif()
else()
    define_aui_boot_test(ViewsProgram)
    define_aui_boot_test(BasicProgram)
    define_aui_boot_test(ImportAUIDependency)
    define_aui_boot_test(Tests)
    define_aui_boot_test(BuildSharedLibsAddSubdirectory)
endif()

# github ci/cd  could not handle this test for unknown reason
# define_aui_boot_test(Assets)
