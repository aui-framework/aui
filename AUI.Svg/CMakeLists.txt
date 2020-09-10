cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Svg)

target_link_libraries(AUI.Svg AUI.Core)
target_link_libraries(AUI.Svg AUI.Views)
target_link_libraries(AUI.Svg AUI.Xml)
