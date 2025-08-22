# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

cmake_minimum_required(VERSION 3.16)

project(AUI)

if (MINGW AND NOT AUI_IGNORE_MINGW)
    message(FATAL_ERROR "MinGW is not supported")
endif()

if(MSVC)
    if (AUI_BUILD_FOR STREQUAL "winxp")    # MT / MTd
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)
    else()                                          # MD / MDd
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL" CACHE STRING "" FORCE)
    endif()
endif()

# AUI_BUILD_AUI_ROOT intended for pulling resources by aui.build.cmake (cmake scripts, ios storyboards, etc...)
set(AUI_BUILD_AUI_ROOT ${CMAKE_CURRENT_SOURCE_DIR} CACHE PATH "AUI src location" FORCE)

# raise aui.boot validation level.
set(AUIB_VALIDATION_LEVEL 2 CACHE STRING "Package validation level")

if (CMAKE_GENERATOR STREQUAL "Unix Makefiles" AND NOT AUI_UNIX_MAKE_FILES_DONT_CARE)
    message(WARNING
            "Please don't use Unix Makefiles otherwise your PC might struggle to build!\n"
            "\n"
            "Use Ninja generator:\n"
            "cmake -GNinja ..\n"
            "Or specify -DAUI_UNIX_MAKE_FILES_DONT_CARE=TRUE:\n"
            "cmake -DAUI_UNIX_MAKE_FILES_DONT_CARE=TRUE ..")
endif ()

if(AUIB_COMPONENTS)
    # adding required dependencies to the components
    list(APPEND AUIB_COMPONENTS core)
    list(APPEND AUIB_COMPONENTS crypt) # toolbox
    list(APPEND AUIB_COMPONENTS image) # toolbox
    if (views IN_LIST AUIB_COMPONENTS)
        list(APPEND AUIB_COMPONENTS xml)
        if (NOT CMAKE_CROSSCOMPILING)
            list(APPEND AUIB_COMPONENTS toolbox)
        endif()
    endif()
endif()
list(REMOVE_DUPLICATES AUIB_COMPONENTS)

include(aui.boot.cmake)

# use msvc dll
cmake_policy(SET CMP0091 NEW)
include(cmake/aui.build.cmake)


set(AUI_BUILD_EXAMPLES false CACHE BOOL "Build AUI examples")
auib_mark_var_forwardable(AUI_BUILD_EXAMPLES)
auib_mark_var_forwardable(AUI_IOS_CODE_SIGNING_REQUIRED)

function(define_aui_component AUI_COMPONENT_NAME)
    if (AUIB_COMPONENTS)
        if (NOT ${AUI_COMPONENT_NAME} IN_LIST AUIB_COMPONENTS)
            return()
        endif()
        message(STATUS "[+] ${AUI_COMPONENT_NAME}")
    endif()
    string(TOUPPER ${AUI_COMPONENT_NAME} AUI_COMPONENT_NAME_UPPER)
    option("AUI_BUILD_${AUI_COMPONENT_NAME_UPPER}" "Build aui::${AUI_COMPONENT_NAME}" ON)
    if (NOT AUI_BUILD_${AUI_COMPONENT_NAME_UPPER})
        return()
    endif()
    add_subdirectory("aui.${AUI_COMPONENT_NAME}")
endfunction()

auib_import(ZLIB https://github.com/aui-framework/zlib
            VERSION 448a9c76a97202a21f68bd82e453dd743e944a3a
            CMAKE_ARGS -DZLIB_BUILD_EXAMPLES=OFF)

# define all components
define_aui_component(core)
if (NOT CMAKE_CROSSCOMPILING)
    define_aui_component(toolbox)
    # define_aui_component(data) -- outdated; need reworking
    define_aui_component(mysql)
    # define_aui_component(data) -- outdated; need reworking
endif()
define_aui_component(network)
define_aui_component(crypt)
define_aui_component(curl)
define_aui_component(image)
define_aui_component(views)
define_aui_component(xml)
define_aui_component(json)
define_aui_component(audio)
define_aui_component(updater)

if (NOT ANDROID AND NOT IOS)
    define_aui_component(uitests)
endif()

if (AUI_BUILD_EXAMPLES) 
    add_subdirectory(examples)
endif()

if (NOT AUI_NO_TESTS)
    if (AUI_BOOT)
        # cleanup tests data when using aui thru aui.boot with add_subdirectory
        set_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS "")
        set_property(GLOBAL PROPERTY TESTS_SRCS "")
        set_property(GLOBAL PROPERTY TESTS_DEPS "")
        _aui_import_gtest()
    endif()
endif()

# all components for exporting
set(AUI_ALL_COMPONENTS core
                       crypt
                       curl
                       data
                       image
                       json
                       mysql
                       network
                       sqlite
                       views
                       xml
                       uitests
                       toolbox
                       audio
                       updater
        )


if(NOT AUIB_COMPONENTS)
    # adding all components
    set(AUIB_COMPONENTS ${AUI_ALL_COMPONENTS})

    # remove unexisting modules from module list (i.e. toolbox)
    foreach(_module ${AUIB_COMPONENTS})
        if (NOT TARGET "aui.${_module}")
            list(REMOVE_ITEM AUIB_COMPONENTS ${_module})
        endif()
    endforeach()
else()
    # remove executables from module list (i.e. toolbox)
    foreach(_module ${AUIB_COMPONENTS})
        if (NOT TARGET "aui.${_module}")
            list(REMOVE_ITEM AUIB_COMPONENTS ${_module})
            message(WARNING "aui.${_module} is either disabled or not exists")
            continue()
        endif()

        get_target_property(_type "aui.${_module}" TYPE)
        if (_type STREQUAL "EXECUTABLE")
            list(REMOVE_ITEM AUIB_COMPONENTS ${_module})
        endif()
    endforeach()
endif()

set(AUI_CONFIG_VARS "set(AUI_ALL_COMPONENTS ${AUI_ALL_COMPONENTS})\nset(AUI_AVAILABLE_COMPONENTS ${AUIB_COMPONENTS})\n")
foreach(_module ${AUIB_COMPONENTS})
    set(_target "aui.${_module}")
    get_target_property(_compile_definitions ${_target} INTERFACE_COMPILE_DEFINITIONS)
    get_target_property(_link_libs ${_target} INTERFACE_LINK_LIBRARIES)
    get_target_property(_wholearchive ${_target} INTERFACE_AUI_WHOLEARCHIVE)
    get_target_property(_link_options ${_target} INTERFACE_LINK_OPTIONS)
    get_target_property(_kind ${_target} TYPE)
    foreach (_kind_variant STATIC SHARED INTERFACE OBJECT)
        if (_kind MATCHES "${_kind_variant}")
            set(_kind ${_kind_variant})
            break()
        endif()
    endforeach()
    unset(_link_libs2)
    foreach(_lib ${_link_libs})
        if (_lib) # skips NOTFOUNDs
            string(REPLACE "aui." "aui::" _lib ${_lib}) # use imported targets instead
            string(REPLACE " " "\\ " _lib ${_lib}) # fixes -framework entries on macos
            list(APPEND _link_libs2 ${_lib})
        endif()
    endforeach()
    set(AUI_CONFIG_VARS "${AUI_CONFIG_VARS}\

set(AUI_COMPONENT_${_module}_COMPILE_DEFINITIONS ${_compile_definitions})
set(AUI_COMPONENT_${_module}_LINK_LIBS ${_link_libs2})
set(AUI_COMPONENT_${_module}_KIND ${_kind})
")
    if (_link_options)
        set(AUI_CONFIG_VARS "${AUI_CONFIG_VARS}\
set(AUI_COMPONENT_${_module}_LINK_OPTIONS ${_link_options})
")
    endif()
    if (_wholearchive)
        set(AUI_CONFIG_VARS "${AUI_CONFIG_VARS}\
set(AUI_COMPONENT_${_module}_WHOLEARCHIVE ON)
")
    endif()
endforeach()


# [auib_precompiled_binary]
auib_precompiled_binary()
# [auib_precompiled_binary]

# test aui.boot on ci/cd
add_subdirectory(test/)

# [configure file example]
configure_file(cmake/aui-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/aui-config.cmake @ONLY)
install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/aui-config.cmake
        DESTINATION ".")
# [configure file example]

install(DIRECTORY cmake/
        DESTINATION cmake)

install(DIRECTORY platform/
        DESTINATION platform)

enable_testing()
