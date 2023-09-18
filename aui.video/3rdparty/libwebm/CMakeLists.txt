cmake_minimum_required(VERSION 3.18)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc)

target_include_directories(aui.video PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(aui.video PRIVATE ${SRCS})