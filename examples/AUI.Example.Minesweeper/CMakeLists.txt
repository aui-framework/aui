cmake_minimum_required(VERSION 3.10)

aui_deploy_library(aui.svg)
aui_deploy_library(aui.image)

aui_executable(AUI.Example.Minesweeper)
aui_compile_assets(AUI.Example.Minesweeper)

target_link_libraries(AUI.Example.Minesweeper PRIVATE aui.core)
target_link_libraries(AUI.Example.Minesweeper PRIVATE aui.views)
