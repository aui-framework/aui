cmake_minimum_required(VERSION 3.10)

project(test_project)

include(${CMAKE_CURRENT_SOURCE_DIR}/../../../../aui.boot.cmake)
auib_import(aui ../../../../
            COMPONENTS core toolbox)

aui_executable(test_project)
aui_link(test_project PUBLIC aui::core)
aui_compile_assets(test_project)