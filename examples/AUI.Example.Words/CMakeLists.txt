cmake_minimum_required(VERSION 3.14)

AUI_Executable(AUI.Example.Words)


target_link_libraries(AUI.Example.Words AUI.Core)
target_link_libraries(AUI.Example.Words AUI.Views)
target_link_libraries(AUI.Example.Words AUI.Svg)
target_link_libraries(AUI.Example.Words AUI.Sqlite)
target_link_libraries(AUI.Example.Words AUI.Json)