cmake_minimum_required(VERSION 3.10)

find_package(CURL)

if (CURL_FOUND)
    AUI_Module(AUI.Curl)
    target_link_libraries(AUI.Curl AUI.Core)
    target_link_libraries(AUI.Curl CURL::libcurl)
else()
    message("cURL library was not found. Disabling AUI.Curl")
endif()