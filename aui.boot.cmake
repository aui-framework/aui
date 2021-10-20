#
# =====================================================================================================================
# Copyright (c) 2021 Alex2772
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated
# documentation files (the "Software"), to deal in the Software without restriction, including without limitation the
# rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
# WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
# OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#
# Original code located at https://github.com/aui-framework/aui
# =====================================================================================================================
#

cmake_minimum_required(VERSION 3.16)

# rpath fix on linux
set(CMAKE_INSTALL_RPATH $ORIGIN)
set(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR}/lib)

define_property(GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES
        BRIEF_DOCS "Global list of aui boot root entries"
        FULL_DOCS "Global list of aui boot root entries")

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

if(WIN32 AND NOT CYGWIN)
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "Please include aui.boot AFTER project() call.")
endif()

if (ANDROID_ABI)
    set(AUI_TARGET_ARCH_NAME "android_${ANDROID_ABI}")
else()
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(AUI_TARGET_ARCH_NAME "x86_64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(AUI_TARGET_ARCH_NAME "x86")
        endif()
    else ()
        set(AUI_TARGET_ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})
    endif ()
endif()

set(AUI_CACHE_DIR ${HOME_DIR}/.aui CACHE PATH "Path to AUI.Boot cache")
string(TOLOWER "${CMAKE_CXX_COMPILER_ID}-${AUI_TARGET_ARCH_NAME}" _tmp)
set(AUI_TARGET_ABI "${_tmp}" CACHE STRING "COMPILER-PROCESSOR pair")
message(STATUS "AUI.Boot cache: ${AUI_CACHE_DIR}")
message(STATUS "AUI.Boot target ABI: ${AUI_TARGET_ABI}")

# create all required dirs
if (NOT EXISTS ${AUI_CACHE_DIR})
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR})
endif()
if (NOT EXISTS ${AUI_CACHE_DIR}/prefix)
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR}/prefix)
endif()
if (NOT EXISTS ${AUI_CACHE_DIR}/repo)
    file(MAKE_DIRECTORY ${AUI_CACHE_DIR}/repo)
endif()

# TODO add a way to provide file access to the repository
macro(auib_import AUI_MODULE_NAME URL)
    cmake_policy(SET CMP0087 NEW)
    cmake_policy(SET CMP0074 NEW)

    set(options)
    set(oneValueArgs VERSION CMAKE_WORKING_DIR)
    set(multiValueArgs CMAKE_ARGS COMPONENTS)
    cmake_parse_arguments(AUIB_IMPORT "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )
    set(TAG_OR_HASH latest)
    if (AUIB_IMPORT_HASH)
        set(TAG_OR_HASH ${AUIB_IMPORT_HASH})
    elseif(AUIB_IMPORT_VERSION)
        set(TAG_OR_HASH ${AUIB_IMPORT_VERSION})
    endif()
    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()
    set(DEP_INSTALL_PREFIX "${AUI_CACHE_DIR}/prefix/${AUI_MODULE_NAME}/${TAG_OR_HASH}/${AUI_TARGET_ABI}/${CMAKE_BUILD_TYPE}")

    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    string(REGEX REPLACE "[a-z]+:\\/\\/" "" URL_PATH ${URL})
    set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}")
    set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    # avoid compilation if we have existing installation
    set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)
    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX})
    if (EXISTS ${DEP_INSTALLED_FLAG})
        if (AUIB_IMPORT_COMPONENTS)
            find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS})
        else()
            find_package(${AUI_MODULE_NAME})
        endif()
    endif()
    if (NOT EXISTS ${DEP_INSTALLED_FLAG} OR NOT ${AUI_MODULE_NAME}_FOUND)
        # some shit with INSTALLED flag because find_package finds by ${AUI_MODULE_NAME}_ROOT only if REQUIRED flag is set
        # so we have to compile and install

        if (NOT EXISTS ${DEP_INSTALLED_FLAG})
            message(STATUS "${AUI_MODULE_NAME}: building because ${DEP_INSTALLED_FLAG} does not exist")
        else()
            message(STATUS "${AUI_MODULE_NAME}: building because find_package could not find package in ${DEP_INSTALL_PREFIX}")
        endif()

        include(FetchContent)
        # TODO add protocol check
        message(STATUS "Fetching ${AUI_MODULE_NAME}")
        # file(LOCK "${AUI_CACHE_DIR}/repo.lock")
        FetchContent_Declare(${AUI_MODULE_NAME}_FC
                GIT_REPOSITORY "${URL}"
                GIT_TAG ${AUIB_IMPORT_VERSION}
                GIT_PROGRESS TRUE # show progress of download
                USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                USES_TERMINAL_UPDATE   TRUE # show progress in ninja generator
                #SOURCE_DIR ${DEP_SOURCE_DIR}
                )

        FetchContent_Populate(${AUI_MODULE_NAME}_FC)


        FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                BINARY_DIR DEP_BINARY_DIR
                SOURCE_DIR DEP_SOURCE_DIR
                )

        message("Fetched ${AUI_MODULE_NAME} to ${DEP_SOURCE_DIR}")

        message(STATUS "Compiling ${AUI_MODULE_NAME}")
        if (AUIB_IMPORT_CMAKE_WORKING_DIR)
            set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
        endif()

        get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
        unset(FORWARDED_LIBS)
        foreach (_entry ${AUI_BOOT_ROOT_ENTRIES})
            list(APPEND FORWARDED_LIBS "-D${_entry}")
        endforeach()
        set(FINAL_CMAKE_ARGS
                -DAUI_BOOT=TRUE
                -DCMAKE_CROSSCOMPILING=${CMAKE_CROSSCOMPILING}
                ${FORWARDED_LIBS}
                ${AUIB_IMPORT_CMAKE_ARGS}
                -DCMAKE_INSTALL_PREFIX:PATH=${DEP_INSTALL_PREFIX}
                -G "${CMAKE_GENERATOR}")

        if (AUIB_IMPORT_COMPONENTS)
            list(JOIN AUIB_IMPORT_COMPONENTS "\\\;" TMP_LIST)
            set(FINAL_CMAKE_ARGS
                    ${FINAL_CMAKE_ARGS}
                    -DAUI_BOOT_COMPONENTS=${TMP_LIST})
        endif()

        # forward all necessary variables to child cmake build
        foreach(_varname CMAKE_SYSTEM_NAME
                         CMAKE_EXPORT_COMPILE_COMMANDS
                         CMAKE_SYSTEM_VERSION
                         ANDROID_PLATFORM
                         ANDROID_ABI
                         CMAKE_ANDROID_ARCH_ABI
                         ANDROID_NDK
                         CMAKE_ANDROID_NDK
                         CMAKE_TOOLCHAIN_FILE
                         CMAKE_MAKE_PROGRAM
                         CMAKE_BUILD_TYPE)
            if (${_varname})
                list(APPEND FINAL_CMAKE_ARGS "-D${_varname}=${${_varname}}")
            endif()
        endforeach()

        message("Building and installing ${AUI_MODULE_NAME}:${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}")
        execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE STATUS_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
        endif()

        message(STATUS "Installing ${AUI_MODULE_NAME}")
        execute_process(COMMAND
                ${CMAKE_COMMAND}
                --build ${DEP_BINARY_DIR}
                --target install

                WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                RESULT_VARIABLE ERROR_CODE)

        if (NOT STATUS_CODE EQUAL 0)
            message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
        endif()
        file(TOUCH ${DEP_INSTALLED_FLAG})
        # file(LOCK "${AUI_CACHE_DIR}/repo.lock" RELEASE)
    endif()
    if (AUIB_IMPORT_COMPONENTS)
        find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS})
    else()
        find_package(${AUI_MODULE_NAME})
    endif()

    if (NOT ${AUI_MODULE_NAME}_FOUND)
        message(FATAL_ERROR "AUI.Boot could not resolve dependency: ${AUI_MODULE_NAME}")
    endif()

    # create links to runtime dependencies
    if (WIN32)
        set(LIB_EXT dll)
    elseif(APPLE)
        set(LIB_EXT dylib)
    else()
        set(LIB_EXT so*)
    endif()
    file(GLOB_RECURSE DEP_RUNTIME LIST_DIRECTORIES false ${DEP_INSTALL_PREFIX}/*.${LIB_EXT})

    if (WIN32)
        set(DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        set(DESTINATION_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    endif()
    file(MAKE_DIRECTORY ${DESTINATION_DIR})

    foreach(_item ${DEP_RUNTIME})
        get_filename_component(FILENAME ${_item} NAME)
        if (CMAKE_CONFIGURATION_TYPES)
            foreach(_config ${CMAKE_CONFIGURATION_TYPES})
                set(_copy ${DESTINATION_DIR}/${_config}/${FILENAME})
                if (NOT EXISTS ${_copy})
                    file(MAKE_DIRECTORY ${DESTINATION_DIR}/${_config})
                    file(COPY ${_item} DESTINATION ${DESTINATION_DIR}/${_config})
                endif()
            endforeach()
        else()
            if (NOT EXISTS ${DESTINATION_DIR}/${FILENAME})
                file(COPY ${_item} DESTINATION ${DESTINATION_DIR})
            endif()
        endif()
    endforeach()
    set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_ROOT=${${AUI_MODULE_NAME}_ROOT}")
endmacro()