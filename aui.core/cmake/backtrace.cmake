cmake_minimum_required(VERSION 3.16)

project(backtrace)

execute_process(COMMAND bash "-c" "./configure --prefix=${CMAKE_INSTALL_PREFIX}"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

execute_process(COMMAND bash "-c" "make"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

execute_process(COMMAND bash "-c" "make install"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

file(WRITE backtrace-config.cmake
"add_library(backtrace STATIC IMPORTED)\n"
"set_target_properties(backtrace PROPERTIES"
"                      IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libbacktrace.a\n"
"                      INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_INSTALL_PREFIX}/include\n)"
)

install(FILES backtrace-config.cmake DESTINATION .)