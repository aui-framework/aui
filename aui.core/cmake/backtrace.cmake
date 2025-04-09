cmake_minimum_required(VERSION 3.16)

project(backtrace)

execute_process(COMMAND bash "-c" "./configure --prefix=${CMAKE_INSTALL_PREFIX} --with-pic"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

execute_process(COMMAND bash "-c" "make"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

execute_process(COMMAND bash "-c" "make install"
                WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

file(WRITE backtrace-config.cmake.in [[
if(TARGET backtrace)
  return()
endif()
add_library(backtrace STATIC IMPORTED)
get_filename_component(_IMPORT_PREFIX "${CMAKE_CURRENT_LIST_FILE}" PATH)
set_target_properties(backtrace PROPERTIES
                      IMPORTED_LOCATION ${_IMPORT_PREFIX}/lib/libbacktrace.a
                      INTERFACE_INCLUDE_DIRECTORIES ${_IMPORT_PREFIX}/include)
]])

configure_file(backtrace-config.cmake.in ${CMAKE_BINARY_DIR}/backtrace-config.cmake @ONLY)

install(FILES ${CMAKE_BINARY_DIR}/backtrace-config.cmake DESTINATION .)