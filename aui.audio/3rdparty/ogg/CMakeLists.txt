cmake_minimum_required(VERSION 3.18)

file(GLOB_RECURSE SRCS src/*.c)

target_include_directories(aui.audio PRIVATE include)
target_include_directories(aui.audio PRIVATE src)
target_sources(aui.audio PRIVATE ${SRCS})