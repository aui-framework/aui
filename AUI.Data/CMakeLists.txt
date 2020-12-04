cmake_minimum_required(VERSION 3.10)

AUI_Module(AUI.Data)

target_link_libraries(AUI.Data PRIVATE AUI.Core)
