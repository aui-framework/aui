# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

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
set_target_properties(backtrace PROPERTIES
                      IMPORTED_LOCATION ${CMAKE_INSTALL_PREFIX}/lib/libbacktrace.a
                      INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_INSTALL_PREFIX}/include)
]])

configure_file(backtrace-config.cmake.in ${CMAKE_BINARY_DIR}/backtrace-config.cmake)

install(FILES ${CMAKE_BINARY_DIR}/backtrace-config.cmake DESTINATION .)