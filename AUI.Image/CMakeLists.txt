cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Image)
target_include_directories(AUI.Image PRIVATE 3rdparty)
target_link_libraries(AUI.Image PRIVATE AUI.Core)
target_link_libraries(AUI.Image PRIVATE AUI.Views)
