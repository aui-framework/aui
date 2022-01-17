cmake_minimum_required(VERSION 3.10)

function(define_aui_boot_test NAME)
    add_test(NAME "AUIBoot.${NAME}"
            COMMAND
            ${CMAKE_COMMAND} -DAUI_TEST_BOOT_BRANCH=${AUI_TEST_BOOT_BRANCH} -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS} -P test.cmake
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${NAME})

endfunction()

define_aui_boot_test(BasicProgram)
define_aui_boot_test(ImportAUIDependency)
define_aui_boot_test(Tests)
