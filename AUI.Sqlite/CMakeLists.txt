cmake_minimum_required(VERSION 3.10)


file(GLOB_RECURSE SQLITE3_SRCS 3rdparty/*.c)

AUI_Module(AUI.Sqlite ${SQLITE3_SRCS})
target_include_directories(AUI.Sqlite PRIVATE 3rdparty/sqlite3)

target_link_libraries(AUI.Sqlite PRIVATE AUI.Core)
target_link_libraries(AUI.Sqlite PRIVATE AUI.Data)
