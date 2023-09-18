cmake_minimum_required(VERSION 3.16)

file(GLOB_RECURSE SRCS source/*.cc)

target_include_directories(aui.video PRIVATE include)
target_include_directories(aui.video PRIVATE source)
target_sources(aui.video PRIVATE ${SRCS})
