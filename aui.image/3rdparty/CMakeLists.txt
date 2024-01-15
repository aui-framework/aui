cmake_minimum_required(VERSION 3.10)

file(GLOB_RECURSE SRCS *.c)
target_include_directories(aui.image PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(aui.image PRIVATE ${SRCS})