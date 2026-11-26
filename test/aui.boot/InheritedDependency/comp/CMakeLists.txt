cmake_minimum_required(VERSION 3.10)
project(comp)

include(${CMAKE_CURRENT_LIST_DIR}/../../../../aui.boot.cmake)

# comp imports base, and its config replays that import, as AUI does with its dependencies
auib_import(base ${CMAKE_CURRENT_LIST_DIR}/../base_v0 VERSION v0)

# only the requested components are installed, as with AUI
foreach (_component ${AUIB_COMPONENTS})
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/${_component}.component "")
    install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${_component}.component DESTINATION components)
endforeach()

set(BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/../base_v0)
configure_file(compConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/compConfig.cmake @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/compConfig.cmake DESTINATION .)
