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

option(AUIB_DISABLE "Disables AUI.Boot and replaces it's calls to find_package" OFF)

define_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS
        BRIEF_DOCS "Global list of imported targets"
        FULL_DOCS "Global list of imported targets (since CMake 3.21)")

# fix "Failed to get the hash for HEAD" error
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
endif()

if (DEFINED BUILD_SHARED_LIBS)
    set(_build_shared ${BUILD_SHARED_LIBS})
else()
    if (ANDROID OR IOS)
        set(_build_shared OFF)
        message(STATUS "Forcing static build because you are building for mobile platform.")
    else()
        set(_build_shared ON)
    endif()
endif()
set(BUILD_SHARED_LIBS ${_build_shared})

if (MSVC)
    if (NOT CMAKE_MSVC_RUNTIME_LIBRARY)
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
        message(STATUS "AUI.Boot CMAKE_MSVC_RUNTIME_LIBRARY is not set - defaulting to ${CMAKE_MSVC_RUNTIME_LIBRARY}")
    endif()
endif()

# rpath fix
if (APPLE)
    set(CMAKE_MACOSX_RPATH 1)
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
elseif(UNIX AND NOT ANDROID)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib")
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    else()
        set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
        set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    endif()
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
    message(FATAL_ERROR "CMAKE_CXX_COMPILER_ID is not set.\nnote: Please include aui.boot AFTER project() call.")
endif()

if (ANDROID_ABI)
    set(AUI_TARGET_ARCH_NAME "android-${ANDROID_ABI}")
    set(AUI_TARGET_ABI "${AUI_TARGET_ARCH_NAME}" CACHE STRING "COMPILER-PROCESSOR pair")
elseif (IOS)
    set(AUI_TARGET_ARCH_NAME "ios-${CMAKE_SYSTEM_PROCESSOR}")
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


macro(_auib_update_imported_targets_list) # used for displaying imported target names
    get_property(_imported_targets_before GLOBAL PROPERTY AUIB_IMPORTED_TARGETS)
    get_property(_imported_targets_after DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    # find the new targets by excluding _imported_targets_before from _imported_targets_after
    list(REMOVE_ITEM _imported_targets_after ${_imported_targets_before})
    list(APPEND _imported_targets_before ${_imported_targets_after})
    set_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS ${_imported_targets_before})
endmacro()

function(_auib_import_subdirectory DEP_SOURCE_DIR AUI_MODULE_NAME) # helper function to keep scope
    set(AUI_BOOT TRUE)
    add_subdirectory(${DEP_SOURCE_DIR} "aui.boot-build-${AUI_MODULE_NAME}")
endfunction()

# TODO add a way to provide file access to the repository
function(auib_import AUI_MODULE_NAME URL)
    if (AUIB_DISABLE)
        if (AUIB_IMPORT_COMPONENTS)
            find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} REQUIRED)
        else()
            find_package(${AUI_MODULE_NAME} REQUIRED)
        endif()
        return()
    endif()

    get_property(AUI_BOOT_IMPORTED_MODULES GLOBAL PROPERTY AUI_BOOT_IMPORTED_MODULES)

    # these constants are needed to ignore case on *nix platforms
    string(TOLOWER "${AUI_MODULE_NAME}" AUI_MODULE_NAME_LOWER)
    string(TOUPPER "${AUI_MODULE_NAME}" AUI_MODULE_NAME_UPPER)

    if (${AUI_MODULE_NAME_LOWER} IN_LIST AUI_BOOT_IMPORTED_MODULES)
        # the module is already imported; skipping
        return()
    endif()

    unset(_local_repo)
    if(EXISTS ${URL})
        # url is a local file
        set(_local_repo TRUE)
        get_filename_component(URL ${URL} ABSOLUTE)
    endif()

    set(_locked FALSE)

    set(FINDPACKAGE_QUIET QUIET)
    if (AUI_BOOT_VERBOSE)
        set(FINDPACKAGE_QUIET "")
    endif()

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

    set(options ADD_SUBDIRECTORY ARCHIVE)
    set(oneValueArgs VERSION CMAKE_WORKING_DIR CMAKELISTS_CUSTOM)
    set(multiValueArgs CMAKE_ARGS COMPONENTS)
    cmake_parse_arguments(AUIB_IMPORT "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    if (AUIB_IMPORT_ARCHIVE AND AUIB_IMPORT_VERSION)
        message(FATAL_ERROR "ARCHIVE and VERSION arguments are incompatible")
    endif()

    if (AUIB_${AUI_MODULE_NAME_UPPER}_AS OR AUIB_ALL_AS OR AUIB_IMPORT_ADD_SUBDIRECTORY)
        set(DEP_ADD_SUBDIRECTORY TRUE)
    else()
        set(DEP_ADD_SUBDIRECTORY FALSE)
    endif()
    if (_local_repo)
        set(URL_PATH ${AUI_MODULE_NAME_LOWER})
    else()
        # find url path by removing url protocol section (i.e. http://)
        string(REGEX REPLACE "[a-z]+:\\/\\/" "" URL_PATH ${URL})
    endif()

    # we need only last dir name from URL_PATH (i.e. aui-framework/aui -> aui)
    string(FIND "${URL_PATH}" "/" _t REVERSE)
    math(EXPR _t "${_t} + 1") # t += 1
    string(SUBSTRING ${URL_PATH} ${_t} -1 URL_PATH)


    set(TAG_OR_HASH latest)
    if (AUIB_IMPORT_ARCHIVE)
        set(TAG_OR_HASH ${URL_PATH})
    elseif (AUIB_IMPORT_HASH)
        set(TAG_OR_HASH ${AUIB_IMPORT_HASH})
    elseif(AUIB_IMPORT_VERSION)
        set(TAG_OR_HASH ${AUIB_IMPORT_VERSION})
    endif()

    # should restrict version length; in order to make equal "f116a123b9b44f362c96632ad5cec980aab8b46c" and "f116a123"
    string(LENGTH ${TAG_OR_HASH} _length)
    if (_length GREATER 16)
        string(SUBSTRING ${TAG_OR_HASH} 0 7 TAG_OR_HASH)
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()

    if (NOT DEFINED BUILD_SHARED_LIBS)
        # default it to ON
        set(BUILD_SHARED_LIBS ON)
    endif()
    if (BUILD_SHARED_LIBS)
        set(SHARED_OR_STATIC shared)
    else()
        set(SHARED_OR_STATIC static)
    endif()
    set(BUILD_SPECIFIER "${TAG_OR_HASH}/${AUI_TARGET_ABI}-${CMAKE_BUILD_TYPE}-${SHARED_OR_STATIC}/${CMAKE_GENERATOR}")

    # convert BUILD_SPECIFIER to hash; on windows msvc path length restricted by 260 chars
    string(MD5 BUILD_SPECIFIER ${BUILD_SPECIFIER})

    # append module name to build specifier in order to distinguish modules in prefix/ dir
    set(BUILD_SPECIFIER "${AUI_MODULE_NAME_LOWER}/${BUILD_SPECIFIER}")

    set(DEP_INSTALL_PREFIX "${AUI_CACHE_DIR}/prefix/${BUILD_SPECIFIER}")

    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()
    endif()

    if (DEP_ADD_SUBDIRECTORY)
        # the AUI_MODULE_NAME is used to hint IDEs (i.e. CLion) about actual project's name
        set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/as/${TAG_OR_HASH}/${AUI_MODULE_NAME_LOWER}")
    else()
        set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/src")
    endif()
    set(DEP_BINARY_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}/build/${BUILD_SPECIFIER}")

    # invalidate all previous values.
    foreach(_v2 FOUND
            INCLUDE_DIR
            LIBRARY
            LIBRARY_DEBUG
            LIBRARY_RELEASE
            ROOT
            DIR)
        foreach(_v1 ${AUI_MODULE_NAME} ${AUI_MODULE_NAME_UPPER})
            unset(${_v1}_${_v2} PARENT_SCOPE)
            unset(${_v1}_${_v2} CACHE)
        endforeach()
    endforeach()

    # variable is CACHE here in order to make it as global as possible.
    set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    # creating uppercase variables in order to ease the case insensitive checks
    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT ${DEP_INSTALL_PREFIX} PARENT_SCOPE)

    set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)

    if (NOT DEP_ADD_SUBDIRECTORY)
        # avoid compilation if we have existing installation
        if (EXISTS ${DEP_INSTALLED_FLAG})
            # BEGIN: try find

            set(CMAKE_FIND_PACKAGE_PREFER_CONFIG FALSE)
            while(TRUE)
                if (AUIB_IMPORT_COMPONENTS)
                    find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET})
                else()
                    find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET})
                endif()
                if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
                    if (CMAKE_FIND_PACKAGE_PREFER_CONFIG)
                        break()
                    endif()
                    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
                else()
                    break()
                endif()
            endwhile()
            unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
            # expose result ignoring case sensitivity
            if (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND)
                set(${AUI_MODULE_NAME}_FOUND TRUE)
            else()
                set(${AUI_MODULE_NAME}_FOUND FALSE)
            endif()
            # END: try find
        endif()
    endif()
    if ((NOT EXISTS ${DEP_INSTALLED_FLAG} OR NOT ${AUI_MODULE_NAME}_FOUND AND NOT DEP_ADD_SUBDIRECTORY) OR ((NOT EXISTS ${DEP_SOURCE_DIR}/CMakeLists.txt) AND DEP_ADD_SUBDIRECTORY))
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

        # TODO add protocol check
        if(AUI_BOOT_SOURCEDIR_COMPAT)
            unset(SOURCE_BINARY_DIRS_ARG)
        else()
            if (NOT AUI_BOOT) # recursive deadlock fix
                if (NOT _locked)
                    set(_locked TRUE)
                    message(STATUS "Waiting for repository...")
                    file(LOCK "${AUI_CACHE_DIR}/repo.lock")
                endif()
            endif()
            set(SOURCE_BINARY_DIRS_ARG SOURCE_DIR ${DEP_SOURCE_DIR}
                    BINARY_DIR ${DEP_BINARY_DIR})
        endif()
        message(STATUS "Fetching ${AUI_MODULE_NAME} (${TAG_OR_HASH})")

        file(REMOVE_RECURSE ${DEP_SOURCE_DIR} ${DEP_BINARY_DIR})

        # check for local existence
        if (EXISTS ${URL})
            get_filename_component(DEP_SOURCE_DIR ${URL} ABSOLUTE)
            message(STATUS "Using local: ${DEP_SOURCE_DIR}")
        else()
            include(FetchContent)

            if (AUIB_IMPORT_ARCHIVE)
                set(_import_type URL)
            else()
                set(_import_type GIT_REPOSITORY)
            endif()

            FetchContent_Declare(${AUI_MODULE_NAME}_FC
                    PREFIX "${CMAKE_BINARY_DIR}/aui.boot-deps/${AUI_MODULE_NAME}"
                    ${_import_type} "${URL}"
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
            message(STATUS "Fetched ${AUI_MODULE_NAME} to ${DEP_SOURCE_DIR}")
        endif()

        if (AUIB_IMPORT_CMAKE_WORKING_DIR)
            set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
        endif()

        if (NOT DEP_ADD_SUBDIRECTORY)
            if (AUIB_IMPORT_CMAKELISTS_CUSTOM)
                configure_file(${AUIB_IMPORT_CMAKELISTS_CUSTOM} ${DEP_SOURCE_DIR}/CMakeLists.txt COPYONLY)
            endif()

            message(STATUS "Compiling ${AUI_MODULE_NAME}")

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
                        -DAUIB_COMPONENTS=${TMP_LIST})
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


            if(MSVC)
                # force msvc compiler to parallel build
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
            endif()

            if (IOS)
                # fix multiple definitions on ios
                set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error-implicit-function-declaration")
            endif()

            # forward all necessary variables to child cmake build
            foreach(_varname
                    AUI_CACHE_DIR
                    CMAKE_C_FLAGS
                    CMAKE_CXX_FLAGS
                    CMAKE_GENERATOR_PLATFORM
                    CMAKE_VS_PLATFORM_NAME
                    CMAKE_BUILD_TYPE
                    CMAKE_CROSSCOMPILING
                    CMAKE_MACOSX_RPATH
                    CMAKE_INSTALL_NAME_DIR
                    CMAKE_INSTALL_RPATH
                    CMAKE_MAKE_PROGRAM
                    CMAKE_MSVC_RUNTIME_LIBRARY
                    CMAKE_FIND_PACKAGE_PREFER_CONFIG
                    CMAKE_FIND_ROOT_PATH_MODE_PROGRAM
                    CMAKE_FIND_ROOT_PATH_MODE_LIBRARY
                    CMAKE_FIND_ROOT_PATH_MODE_INCLUDE
                    CMAKE_FIND_ROOT_PATH_MODE_PACKAGE
                    ONLY_CMAKE_FIND_ROOT_PATH
                    ${ANDROID_VARS})

                # ${_varname} can be possibly false (e.g. -DBUILD_SHARED_LIBS=FALSE) so using STREQUAL check instead for
                # emptiness
                if (NOT ${_varname} STREQUAL "")
                    list(APPEND FINAL_CMAKE_ARGS "-D${_varname}=${${_varname}}")
                endif()
            endforeach()
            if (CMAKE_TOOLCHAIN_FILE) # resolve absolute path to the toolchain file - it's possibly relative thus invalid
                get_filename_component(_toolchain ${CMAKE_TOOLCHAIN_FILE} ABSOLUTE)
                list(APPEND FINAL_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${_toolchain}")
            endif()
            list(APPEND FINAL_CMAKE_ARGS "-DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS}")

            message("Configuring CMake ${AUI_MODULE_NAME}:${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}")
            execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                    WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                    RESULT_VARIABLE STATUS_CODE)

            if (NOT STATUS_CODE EQUAL 0)
                message(STATUS "Dependency CMake configure failed, clearing dir and trying again...")
                file(REMOVE_RECURSE ${DEP_BINARY_DIR})
                file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE)
                if (NOT STATUS_CODE EQUAL 0)
                    message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
                endif()
            endif()

            message(STATUS "Building ${AUI_MODULE_NAME}")

            set(_flags "")
            if (NOT MSVC)
                LIST(APPEND _flags "--parallel") # --parallel breaks multithread build for MSVC
            endif()

            execute_process(COMMAND
                    ${CMAKE_COMMAND}
                    --build ${DEP_BINARY_DIR} ${_flags}
                    --config ${CMAKE_BUILD_TYPE} # fix vs and xcode generators

                    WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                    RESULT_VARIABLE ERROR_CODE)

            if (NOT STATUS_CODE EQUAL 0)
                message(FATAL_ERROR "Dependency build failed: ${AUI_MODULE_NAME}")
            endif()

            message(STATUS "Installing ${AUI_MODULE_NAME}")
            execute_process(COMMAND
                    ${CMAKE_COMMAND}
                    --install .
                    --config ${CMAKE_BUILD_TYPE} # fix vs and xcode generators

                    WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                    RESULT_VARIABLE ERROR_CODE)

            if (NOT STATUS_CODE EQUAL 0)
                message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
            endif()
            if (NOT EXISTS ${DEP_INSTALL_PREFIX})
                message(FATAL_ERROR "Dependency failed to install: ${AUI_MODULE_NAME} - check the compilation and installation logs above")
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
        set(${AUI_MODULE_NAME}_ROOT ${DEP_SOURCE_DIR})
        if (AUIB_IMPORT_CMAKELISTS_CUSTOM)
            configure_file(${AUIB_IMPORT_CMAKELISTS_CUSTOM} ${DEP_SOURCE_DIR}/CMakeLists.txt COPYONLY)
        endif()
        _auib_import_subdirectory(${DEP_SOURCE_DIR} ${AUI_MODULE_NAME})
        message(STATUS "${AUI_MODULE_NAME} imported as a subdirectory: ${DEP_SOURCE_DIR}")
    else()

        # BEGIN: try find
        set(CMAKE_FIND_PACKAGE_PREFER_CONFIG FALSE)
        while(TRUE)
            if (AUIB_IMPORT_COMPONENTS)
                find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET})
            else()
                find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET})
            endif()
            if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
                if (CMAKE_FIND_PACKAGE_PREFER_CONFIG)
                    break()
                endif()
                set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
            else()
                break()
            endif()
        endwhile()
        unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
        # expose result ignoring case sensitivity
        if (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND)
            set(${AUI_MODULE_NAME}_FOUND TRUE)
        else()
            set(${AUI_MODULE_NAME}_FOUND FALSE)
        endif()
        # END: try find
        if (NOT ${AUI_MODULE_NAME}_FOUND)
            # print verbosely find procedure

            set(CMAKE_FIND_DEBUG_MODE TRUE)
            # BEGIN: try find

            set(CMAKE_FIND_PACKAGE_PREFER_CONFIG FALSE)
            message("[AUI.BOOT] Verbose output:")
            while(TRUE)
                if (AUIB_IMPORT_COMPONENTS)
                    find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET})
                else()
                    find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET})
                endif()
                if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
                    if (CMAKE_FIND_PACKAGE_PREFER_CONFIG)
                        message("[AUI.BOOT] Dependency not found - giving up")
                        break()
                    endif()
                    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
                    message("[AUI.BOOT] Using config instead")
                else()
                    break()
                endif()
            endwhile()
            unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)

            # list possible find_package names if available
            file(GLOB_RECURSE _find "${DEP_INSTALL_PREFIX}/*onfig.cmake")
            unset(possible_names)
            foreach(_i ${_find})
                get_filename_component(_name ${_i} NAME)
                string(REGEX REPLACE "-?([Cc]onfig)\\.cmake" "" _name ${_name})
                list(APPEND possible_names "\"${_name}\"")
            endforeach()

            # construct error message
            set(error_message "AUI.Boot could not resolve dependency: ${AUI_MODULE_NAME} (see verbose find output above)")
            set(error_message "${error_message}\nnote: package names are case sensitive")
            if (possible_names)
                string(JOIN " or " possible_names_joined ${possible_names})
                set(error_message "${error_message}\nnote: did you mean " ${possible_names_joined} ?)
            else()
                set(error_message "${error_message}\nnote: looks like a config file does not exist for your project (${AUI_MODULE_NAME}Config.cmake or ${AUI_MODULE_NAME}-config.cmake).")
            endif()
            message(FATAL_ERROR ${error_message})
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
                set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
            endif()

            set(DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
        else()
            # sometimes it's empty
            if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
                set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
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
                    message(VERBOSE "[AUI.BOOT] ${_item} -> ${DESTINATION_DIR}/${FILENAME}")
                    file(COPY ${_item} DESTINATION ${DESTINATION_DIR})
                endif()
            endif()
        endforeach()
    endif()

    set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_ROOT=${${AUI_MODULE_NAME}_ROOT}")
    set_property(GLOBAL APPEND PROPERTY AUI_BOOT_IMPORTED_MODULES ${AUI_MODULE_NAME_LOWER})

    # display the imported targets (available since CMake 3.21)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()
        message(STATUS "Imported: ${AUI_MODULE_NAME} (${_imported_targets_after}) (${${AUI_MODULE_NAME}_ROOT})")
    else()
        message(STATUS "Imported: ${AUI_MODULE_NAME} (${${AUI_MODULE_NAME}_ROOT})")
    endif()

endfunction()


macro(auib_use_system_libs_begin)
    set(AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
endmacro()

macro(auib_use_system_libs_end)
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
endmacro()