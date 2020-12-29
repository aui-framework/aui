cmake_minimum_required(VERSION 3.10)

AUI_Executable(AUI.Example.Views)
AUI_Compile_Assets(AUI.Example.Views)

target_link_libraries(AUI.Example.Views PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Views PRIVATE AUI.Views)
target_link_libraries(AUI.Example.Views PRIVATE AUI.Svg)
