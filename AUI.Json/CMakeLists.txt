cmake_minimum_required(VERSION 3.10)

aui_module(AUI.Json)

target_link_libraries(AUI.Json PRIVATE AUI.Core)
target_link_libraries(AUI.Json PRIVATE AUI.Data)