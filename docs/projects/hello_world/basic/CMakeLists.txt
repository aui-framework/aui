# Standard routine
cmake_minimum_required(VERSION 3.16)
project(project_template)

# Link AUI
find_package(AUI REQUIRED)

# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(project_template)

# Link required libs
target_link_libraries(project_template PRIVATE AUI.Core)
# target_link_libraries(project_template PRIVATE AUI.Views)