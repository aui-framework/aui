cmake_minimum_required(VERSION 3.10)

AUI_Deploy_Library(AUI.Svg)
AUI_Deploy_Library(AUI.Image)

AUI_Executable(AUI.Example.Minesweeper)
AUI_Compile_Assets(AUI.Example.Minesweeper)

target_link_libraries(AUI.Example.Minesweeper PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Minesweeper PRIVATE AUI.Views)
