cmake_minimum_required(VERSION 3.10)

aui_deploy_library(AUI.Svg)
aui_deploy_library(AUI.Image)

aui_executable(AUI.Example.Fractal)
aui_compile_assets(AUI.Example.Fractal)

target_link_libraries(AUI.Example.Fractal PRIVATE AUI.Core)
target_link_libraries(AUI.Example.Fractal PRIVATE AUI.Views)
