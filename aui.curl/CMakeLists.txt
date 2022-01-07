cmake_minimum_required(VERSION 3.10)

auib_import(CURL https://github.com/curl/curl
            VERSION curl-7_79_1
            CMAKE_ARGS -DCMAKE_USE_OPENSSL=ON)

aui_module(aui.curl EXPORT aui)
target_link_libraries(aui.curl PUBLIC aui.core)
target_link_libraries(aui.curl PRIVATE CURL::libcurl)