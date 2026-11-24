cmake_minimum_required(VERSION 3.10)
project(mid)

# the copy vendored by mid; test.cmake puts it here
include(${CMAKE_CURRENT_LIST_DIR}/aui.boot.cmake)

# mid pins its own version of base; the parent's base must win
auib_import(base ${CMAKE_CURRENT_LIST_DIR}/../base_v0 VERSION v0)

add_library(mid STATIC mid.cpp)
add_library(mid::mid ALIAS mid)
target_link_libraries(mid PUBLIC base::base)

install(TARGETS mid EXPORT mid ARCHIVE DESTINATION lib)
install(EXPORT mid DESTINATION cmake NAMESPACE mid:: FILE midConfig.cmake)
