cmake_minimum_required(VERSION 3.14)

AUI_Executable(tests)

find_package(Boost REQUIRED)

target_include_directories(tests PRIVATE ${Boost_INCLUDE_DIRS})

target_link_libraries(tests AUI.Core)
target_link_libraries(tests AUI.Data)
target_link_libraries(tests AUI.Views)
target_link_libraries(tests AUI.Crypt)
target_link_libraries(tests AUI.Svg)
target_link_libraries(tests AUI.Sqlite)

add_test(NAME tests COMMAND tests)