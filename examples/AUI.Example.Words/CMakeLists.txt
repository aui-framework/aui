cmake_minimum_required(VERSION 3.10)

AUI_Executable(AUI.Example.Words)
AUI_Compile_Assets(AUI.Example.Words)

target_link_libraries(AUI.Example.Words PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Words PRIVATE AUI.Views)
target_link_libraries(AUI.Example.Words PRIVATE AUI.Svg)
target_link_libraries(AUI.Example.Words PRIVATE AUI.Sqlite)
target_link_libraries(AUI.Example.Words PRIVATE AUI.Json)
