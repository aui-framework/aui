cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Svg)

target_include_directories(AUI.Svg PRIVATE 3rdparty/nanosvg/src)

target_link_libraries(AUI.Svg PRIVATE AUI.Core)
target_link_libraries(AUI.Svg PRIVATE AUI.Views)
target_link_libraries(AUI.Svg PRIVATE AUI.Xml)
