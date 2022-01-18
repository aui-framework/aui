cmake_minimum_required(VERSION 3.10)

project(test_project)

file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_TEST_BOOT_BRANCH}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
auib_import(AUI https://github.com/aui-framework/aui
            VERSION ${AUI_TEST_BOOT_BRANCH}
            COMPONENTS core views)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core aui::views)
aui_compile_assets(test_project)