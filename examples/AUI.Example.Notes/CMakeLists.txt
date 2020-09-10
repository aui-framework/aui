cmake_minimum_required(VERSION 3.10)

AUI_Executable(AUI.Example.Notes)
AUI_Compile_Assets(AUI.Example.Notes)

target_link_libraries(AUI.Example.Notes AUI.Core)
target_link_libraries(AUI.Example.Notes AUI.Views)
target_link_libraries(AUI.Example.Notes AUI.Svg)
target_link_libraries(AUI.Example.Notes AUI.Sqlite)
target_link_libraries(AUI.Example.Notes AUI.Json)
