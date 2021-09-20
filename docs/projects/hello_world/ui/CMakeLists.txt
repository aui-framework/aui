cmake_minimum_required(VERSION 3.16)
project(graphical_example)

# Link AUI
find_package(AUI REQUIRED)

# Create the executable. This function automatically links all sources from the src/ folder, creates CMake target and
# places the resulting executable to bin/ folder.
aui_executable(graphical_example)

# Link required libs
target_link_libraries(graphical_example PRIVATE AUI.Core)
target_link_libraries(graphical_example PRIVATE AUI.Views)