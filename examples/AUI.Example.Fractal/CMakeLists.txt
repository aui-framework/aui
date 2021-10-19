cmake_minimum_required(VERSION 3.10)

aui_executable(AUI.Example.Fractal)
aui_compile_assets(AUI.Example.Fractal)

target_link_libraries(AUI.Example.Fractal PRIVATE aui.core)
target_link_libraries(AUI.Example.Fractal PRIVATE aui.views)
