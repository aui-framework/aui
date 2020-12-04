cmake_minimum_required(VERSION 3.10)


if (TARGET AUI.Views)
AUI_Module(AUI.Svg)

target_include_directories(AUI.Svg PRIVATE 3rdparty/nanosvg/src)

target_link_libraries(AUI.Svg PRIVATE AUI.Core)
target_link_libraries(AUI.Svg PRIVATE AUI.Views)
target_link_libraries(AUI.Svg PRIVATE AUI.Xml)
else()
    message("Could not find AUI.Views; Disabling AUI.Svg")
endif()
