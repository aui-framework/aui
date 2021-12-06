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

# rpath fix
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
elseif(UNIX)
    set(CMAKE_INSTALL_RPATH $ORIGIN)
    set(CMAKE_BUILD_RPATH ${CMAKE_BINARY_DIR}/lib)
endif()

define_property(GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES
        BRIEF_DOCS "Global list of aui boot root entries"
        FULL_DOCS "Global list of aui boot root entries")

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

# checking host system not by WIN32 because of cross compilation
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "Please include aui.boot AFTER project() call.")
endif()

if (ANDROID_ABI)
    set(AUI_TARGET_ARCH_NAME "android-${ANDROID_ABI}")
    set(AUI_TARGET_ABI "${AUI_TARGET_ARCH_NAME}" CACHE STRING "COMPILER-PROCESSOR pair")
else()
    if (NOT CMAKE_SYSTEM_PROCESSOR)
        message(FATAL_ERROR "CMAKE_SYSTEM_PROCESSOR is not set")
    endif()
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(AUI_TARGET_ARCH_NAME "x86_64")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
            set(AUI_TARGET_ARCH_NAME "x86")
        endif()
    else ()
        set(AUI_TARGET_ARCH_NAME ${CMAKE_SYSTEM_PROCESSOR})
    endif()
    string(TOLOWER "${CMAKE_CXX_COMPILER_ID}-${AUI_TARGET_ARCH_NAME}" _tmp)
    set(AUI_TARGET_ABI "${_tmp}" CACHE STRING "COMPILER-PROCESSOR pair")
endif()

set(AUI_CACHE_DIR ${HOME_DIR}/.aui CACHE PATH "Path to AUI.Boot cache")
message(STATUS "AUI.Boot cache: ${AUI_CACHE_DIR}")
message(STATUS "AUI.Boot target ABI: ${AUI_TARGET_ABI}")



set(AUI_BOOT_SOURCEDIR_COMPAT OFF)
if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    message(STATUS "Dependencies will be cloned to build directory, not to ${AUI_CACHE_DIR} because you're using CMake older"
            " than 3.21. See https://github.com/aui-framework/aui/issues/6 for details.")
    set(AUI_BOOT_SOURCEDIR_COMPAT ON)
endif()


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
    set(_error_level STATUS) # we'll try for a second time if we encounter some errors
    while(TRUE)
        cmake_policy(SET CMP0087 NEW)
        cmake_policy(SET CMP0074 NEW)
        # https://stackoverflow.com/a/46057018
        if (CMAKE_CROSSCOMPILING)
            set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
            set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
            set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
            set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE BOTH)
            set(ONLY_CMAKE_FIND_ROOT_PATH TRUE)
        endif()
        set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH FALSE)

        set(options ADD_SUBDIRECTORY)
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
        set(BUILD_SPECIFIER "${AUI_MODULE_NAME}/${TAG_OR_HASH}/${AUI_TARGET_ABI}/${CMAKE_BUILD_TYPE}")
        set(DEP_INSTALL_PREFIX "${AUI_CACHE_DIR}/prefix/${BUILD_SPECIFIER}")

        # append our location to module path
        #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
        #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
        #endif()

        if (AUI_BOOT_${AUI_MODULE_NAME}_ADD_SUBDIRECTORY OR AUIB_IMPORT_ADD_SUBDIRECTORY)
            set(DEP_ADD_SUBDIRECTORY TRUE)
        else()
            set(DEP_ADD_SUBDIRECTORY FALSE)
        endif()
        string(REGEX REPLACE "[a-z]+:\\/\\/" "" URL_PATH ${URL})

        if (DEP_ADD_SUBDIRECTORY)
            # the AUI_MODULE_NAME is used to hint IDEs (i.e. CLion) about actual project's name
            set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/as/${TAG_OR_HASH}/${AUI_MODULE_NAME}")
        else()
            set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/src")
        endif()
        set(DEP_BINARY_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/build/${BUILD_SPECIFIER}/${CMAKE_GENERATOR}")
        set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

        set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)
        if (NOT DEP_ADD_SUBDIRECTORY)
            # avoid compilation if we have existing installation
            set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX})
            if (EXISTS ${DEP_INSTALLED_FLAG})
                if (AUIB_IMPORT_COMPONENTS)
                    find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS})
                else()
                    find_package(${AUI_MODULE_NAME})
                endif()
            endif()
        endif()
        if (((NOT EXISTS ${DEP_INSTALLED_FLAG} OR NOT ${AUI_MODULE_NAME}_FOUND) AND NOT DEP_ADD_SUBDIRECTORY) OR ((NOT EXISTS ${DEP_SOURCE_DIR}/CMakeLists.txt) AND DEP_ADD_SUBDIRECTORY))
            # some shit with INSTALLED flag because find_package finds by ${AUI_MODULE_NAME}_ROOT only if REQUIRED flag is set
            # so we have to compile and install
            if (NOT DEP_ADD_SUBDIRECTORY)
                if (NOT EXISTS ${DEP_INSTALLED_FLAG})
                    message(STATUS "${AUI_MODULE_NAME}: building because ${DEP_INSTALLED_FLAG} does not exist")
                else()
                    message(STATUS "${AUI_MODULE_NAME}: building because find_package could not find package in ${DEP_INSTALL_PREFIX}")
                endif()
            else()
                if (EXISTS ${DEP_SOURCE_DIR})
                    file(REMOVE_RECURSE ${DEP_SOURCE_DIR})
                endif()
            endif()

            include(FetchContent)
            # TODO add protocol check
            if(AUI_BOOT_SOURCEDIR_COMPAT)
                unset(SOURCE_BINARY_DIRS_ARG)
            else()
                if (NOT AUI_BOOT) # recursive deadlock fix
                    message(STATUS "Waiting for repository...")
                    file(LOCK "${AUI_CACHE_DIR}/repo.lock")
                endif()
                set(SOURCE_BINARY_DIRS_ARG SOURCE_DIR ${DEP_SOURCE_DIR}
                        BINARY_DIR ${DEP_BINARY_DIR})
            endif()
            message(STATUS "Fetching ${AUI_MODULE_NAME}")

            file(REMOVE_RECURSE ${DEP_SOURCE_DIR} ${DEP_BINARY_DIR})
            FetchContent_Declare(${AUI_MODULE_NAME}_FC
                    GIT_REPOSITORY "${URL}"
                    GIT_TAG ${AUIB_IMPORT_VERSION}
                    GIT_PROGRESS TRUE # show progress of download
                    USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                    USES_TERMINAL_UPDATE   TRUE # show progress in ninja generator
                    ${SOURCE_BINARY_DIRS_ARG}
                    )

            FetchContent_Populate(${AUI_MODULE_NAME}_FC)


            FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                    BINARY_DIR DEP_BINARY_DIR
                    SOURCE_DIR DEP_SOURCE_DIR
                    )

            message("Fetched ${AUI_MODULE_NAME} to ${DEP_SOURCE_DIR}")

            if (NOT DEP_ADD_SUBDIRECTORY)
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

                if(ANDROID)
                    set(ANDROID_VARS
                            CMAKE_SYSTEM_NAME
                            CMAKE_EXPORT_COMPILE_COMMANDS
                            CMAKE_SYSTEM_VERSION
                            ANDROID_PLATFORM
                            ANDROID_ABI
                            CMAKE_ANDROID_ARCH_ABI
                            ANDROID_NDK
                            CMAKE_ANDROID_NDK
                            )
                endif()

                # fix: convert CMAKE_TOOLCHAIN_FILE to absolute path
                if (CMAKE_TOOLCHAIN_FILE)
                    get_filename_component(ABSOLUTE_PATH ${CMAKE_TOOLCHAIN_FILE} CMAKE_TOOLCHAIN_FILE)
                endif()

                # forward all necessary variables to child cmake build
                foreach(_varname
                        CMAKE_TOOLCHAIN_FILE
                        CMAKE_GENERATOR_PLATFORM
                        CMAKE_VS_PLATFORM_NAME
                        CMAKE_BUILD_TYPE
                        CMAKE_CROSSCOMPILING
                        CMAKE_MACOSX_RPATH
                        CMAKE_INSTALL_NAME_DIR
                        CMAKE_INSTALL_RPATH
                        CMAKE_MAKE_PROGRAM
                        ${ANDROID_VARS})
                    if (${_varname})
                        list(APPEND FINAL_CMAKE_ARGS "-D${_varname}=${${_varname}}")
                    endif()
                endforeach()

                message("Building and installing ${AUI_MODULE_NAME}:${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}")
                execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE)

                if (NOT STATUS_CODE EQUAL 0)
                    message(STATUS "CMake configure failed, clearing dir and trying again...")
                    file(REMOVE_RECURSE ${DEP_BINARY_DIR})
                    file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                    execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                            WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                            RESULT_VARIABLE STATUS_CODE)
                    if (NOT STATUS_CODE EQUAL 0)
                        message(${_error_level} "CMake configure failed: ${STATUS_CODE}")

                        # try for a second time
                        set(${AUI_MODULE_NAME}_FC_POPULATED FALSE) # trick fetchcontent
                        set(_error_level FATAL_ERROR)
                        continue()
                    endif()
                endif()

                message(STATUS "Installing ${AUI_MODULE_NAME}")
                execute_process(COMMAND
                        ${CMAKE_COMMAND}
                        --build ${DEP_BINARY_DIR}
                        --target install

                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE ERROR_CODE)

                if (NOT STATUS_CODE EQUAL 0)
                    message(${_error_level} "CMake build failed: ${STATUS_CODE}")

                    # try for a second time
                    set(${AUI_MODULE_NAME}_FC_POPULATED FALSE) # trick fetchcontent
                    set(_error_level FATAL_ERROR)
                    continue()
                endif()
                file(TOUCH ${DEP_INSTALLED_FLAG})
            endif()
            if (NOT AUI_BOOT_SOURCEDIR_COMPAT)
                if (NOT AUI_BOOT) # recursive deadlock fix
                    file(LOCK "${AUI_CACHE_DIR}/repo.lock" RELEASE)
                endif()
            endif()
        endif()
        if (DEP_ADD_SUBDIRECTORY)
            message(STATUS "${AUI_MODULE_NAME} imported as a subdirectory: ${DEP_SOURCE_DIR}")
            add_subdirectory(${DEP_SOURCE_DIR} "aui.boot-build-${AUI_MODULE_NAME}")
        else()
            if (AUIB_IMPORT_COMPONENTS)
                find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS})
            else()
                find_package(${AUI_MODULE_NAME})
            endif()

            if (NOT ${AUI_MODULE_NAME}_FOUND)
                message(${_error_level} "AUI.Boot could not resolve dependency: ${AUI_MODULE_NAME}")

                # try for a second time
                set(${AUI_MODULE_NAME}_FC_POPULATED FALSE) # trick fetchcontent
                set(_error_level FATAL_ERROR)
                continue()
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
                # sometimes it's empty
                if (NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
                    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
                endif()

                set(DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
            else()
                # sometimes it's empty
                if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
                    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
                endif()

                set(DESTINATION_DIR ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
            endif()

            if (NOT EXISTS ${DESTINATION_DIR})
                file(MAKE_DIRECTORY ${DESTINATION_DIR})
            endif()

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
        endif()
        break()
    endwhile()
endmacro()