cmake_minimum_required(VERSION 3.14)

AUI_Module(AUI.Curl)

target_link_libraries(AUI.Curl AUI.Core)

find_package(CURL)
target_link_libraries(AUI.Curl CURL::libcurl)