cmake_minimum_required(VERSION 3.10)

auib_import(CURL https://github.com/curl/curl
            VERSION curl-7_79_1)

aui_module(aui.curl)
target_link_libraries(aui.curl PUBLIC aui.core)
target_link_libraries(aui.curl PRIVATE CURL::libcurl)