cmake_minimum_required(VERSION 3.10)

aui_executable(AUI.Example.Views)
aui_compile_assets(AUI.Example.Views)


target_link_libraries(AUI.Example.Views PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Views PRIVATE AUI.Views)
target_link_libraries(AUI.Example.Views PRIVATE AUI.Svg)
target_link_libraries(AUI.Example.Views PRIVATE AUI.Image)
