cmake_minimum_required(VERSION 3.10)

aui_executable(AUI.Example.Views)
aui_compile_assets(AUI.Example.Views)


target_link_libraries(AUI.Example.Views PRIVATE aui.core)
target_link_libraries(AUI.Example.Views PRIVATE aui.views)
target_link_libraries(AUI.Example.Views PRIVATE aui.svg)
target_link_libraries(AUI.Example.Views PRIVATE aui.image)
