cmake_minimum_required(VERSION 3.10)

project(test_library)

if (NOT DEFINED BUILD_SHARED_LIBS)
    message(FATAL_ERROR "BUILD_SHARED_LIBS IS NOT DEFINED")
endif()
if (BUILD_SHARED_LIBS)
    message(FATAL_ERROR "BUILD_SHARED_LIBS IS ${BUILD_SHARED_LIBS}")
endif()

add_library(test_library src/main.cpp)

install(TARGETS test_library
        EXPORT test_library
        ARCHIVE       DESTINATION "lib"
        LIBRARY       DESTINATION "lib"
        RUNTIME       DESTINATION "bin"
        PUBLIC_HEADER DESTINATION "include"
        INCLUDES      DESTINATION "include")

install(EXPORT test_library DESTINATION "cmake" FILE test_libraryConfig.cmake)