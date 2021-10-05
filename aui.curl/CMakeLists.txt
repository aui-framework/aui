cmake_minimum_required(VERSION 3.10)

find_package(CURL)

if (CURL_FOUND)
    aui_module(aui.curl)
    target_link_libraries(aui.curl PUBLIC aui.core)
else()
    message(INFO "cURL is not available, disabling aui.curl")
endif()