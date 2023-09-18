cmake_minimum_required(VERSION 3.16)

target_include_directories(aui.video PRIVATE ./)
target_include_directories(aui.video PRIVATE ./configured-build)

find_library(LIBVPX vpx ./configured-build)
target_link_libraries(aui.video PRIVATE ${LIBVPX})
target_sources(aui.video PRIVATE tools_common.c)
