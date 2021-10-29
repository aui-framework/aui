# Standard routine
cmake_minimum_required(VERSION 3.16)
project(project_template)

# Use AUI.Boot
file(
    DOWNLOAD 
    https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake 
    ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

# link AUI
auib_import(
    AUI https://github.com/aui-framework/aui 
    COMPONENTS core)


# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(project_template)

# Link required libs
target_link_libraries(project_template PRIVATE aui::core)
