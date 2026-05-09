# Standard routine
cmake_minimum_required(VERSION 3.16)
project(project_template)

# Use AUI.Boot
#
# Download aui.boot.cmake (one-time):
#   curl https://raw.githubusercontent.com/aui-framework/aui/refs/heads/develop/aui.boot.cmake -o aui.boot.cmake
#
# Or use CLI mode:
#   cmake -P aui.boot.cmake update
#
include(aui.boot.cmake)

# link AUI
auib_import(
    AUI https://github.com/aui-framework/aui 
    COMPONENTS core)


# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(project_template)

# Link required libs
target_link_libraries(project_template PRIVATE aui::core)
