cmake_minimum_required(VERSION 3.10)

AUI_Executable(AUI.Example.Notes)
AUI_Compile_Assets(AUI.Example.Notes)

target_link_libraries(AUI.Example.Notes PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Notes PRIVATE AUI.Views)
target_link_libraries(AUI.Example.Notes PRIVATE AUI.Svg)
target_link_libraries(AUI.Example.Notes PRIVATE AUI.Sqlite)
target_link_libraries(AUI.Example.Notes PRIVATE AUI.Json)
