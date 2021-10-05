cmake_minimum_required(VERSION 3.10)


file(GLOB_RECURSE SQLITE3_SRCS 3rdparty/*.c)

aui_module(aui.sqlite ${SQLITE3_SRCS})
target_include_directories(aui.sqlite PRIVATE 3rdparty/sqlite3)

target_link_libraries(aui.sqlite PRIVATE aui.core)
target_link_libraries(aui.sqlite PRIVATE aui.data)
