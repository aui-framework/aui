cmake_minimum_required(VERSION 3.14)

AUI_Executable(AUI.Example.Views)
AUI_Compile_Assets(AUI.Example.Views)

target_link_libraries(AUI.Example.Views AUI.Core)
target_link_libraries(AUI.Example.Views AUI.Views)
target_link_libraries(AUI.Example.Views AUI.Svg)