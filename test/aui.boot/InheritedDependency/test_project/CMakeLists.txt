cmake_minimum_required(VERSION 3.10)
project(test_project)

include(../../../../aui.boot.cmake)

auib_import(base ${CMAKE_CURRENT_SOURCE_DIR}/../base_${TEST_BASE_VERSION} VERSION ${TEST_BASE_VERSION})
if (TEST_NO_REQUIRES)
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid)
else()
    auib_import(mid ${CMAKE_CURRENT_SOURCE_DIR}/../mid REQUIRES base)
endif()

add_executable(test_project main.cpp)
target_link_libraries(test_project PRIVATE mid::mid)
