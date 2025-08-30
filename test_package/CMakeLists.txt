cmake_minimum_required(VERSION 3.10)

project(test_project)

find_package(aui REQUIRED)

aui_executable(${PROJECT_NAME})
aui_link(${PROJECT_NAME} PUBLIC aui::core)
aui_compile_assets(${PROJECT_NAME})
