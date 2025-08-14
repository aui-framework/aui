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

find_program(GIT_EXECUTABLE NAMES git git.cmd)
if (NOT GIT_EXECUTABLE)
    message(FATAL_ERROR "[AUI.BOOT/Git Check] Git not found! Please install Git and try again. https://git-scm.com/")
endif ()

define_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS
        BRIEF_DOCS "Global list of imported targets"
        FULL_DOCS "Global list of imported targets (since CMake 3.21)")

define_property(GLOBAL PROPERTY AUIB_FORWARDABLE_VARS
        BRIEF_DOCS "Global list of forwarded vars"
        FULL_DOCS "Global list of forwarded vars")

macro(auib_mark_var_forwardable VAR)
    set_property(GLOBAL APPEND PROPERTY AUIB_FORWARDABLE_VARS ${VAR})
endmacro()

option(AUIB_NO_PRECOMPILED "Forbid usage of precompiled packages")
option(AUIB_FORCE_PRECOMPILED "Forbid local build and use precompiled packages only")
option(AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT "install dependencies managed with AUIB_DEPS inside of your package" OFF)
option(AUIB_DISABLE "Disables AUI.Boot and replaces it's calls to find_package" OFF)
option(AUIB_LOCAL_CACHE "Redirects AUI.Boot cache dir from the home directory to CMAKE_BINARY_DIR/aui.boot" OFF)
option(CMAKE_POSITION_INDEPENDENT_CODE "Use position independent code (-fPIC). Enabled by default for compatibility and security reasons." ON)
set(AUIB_VALIDATION_LEVEL 1 CACHE STRING "Package validation level")

auib_mark_var_forwardable(AUIB_NO_PRECOMPILED)
auib_mark_var_forwardable(AUIB_FORCE_PRECOMPILED)
auib_mark_var_forwardable(AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
auib_mark_var_forwardable(AUIB_DISABLE)
auib_mark_var_forwardable(AUIB_LOCAL_CACHE)
auib_mark_var_forwardable(AUIB_VALIDATION_LEVEL)

if (AUIB_NO_PRECOMPILED AND AUIB_FORCE_PRECOMPILED)
    message(FATAL_ERROR "AUIB_NO_PRECOMPILED and AUIB_FORCE_PRECOMPILED are exclusive.")
endif()

function(_auib_fix_multiconfiguration)
    get_property(_tmp GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
    if (NOT _tmp)
        return()
    endif()
    message(STATUS "\nMulti configuration generator detected (https://github.com/aui-framework/aui/issues/133)"
            "\nPlease use ninja generator if possible")

    if (CMAKE_BUILD_TYPE)
        message(STATUS "CMAKE_CONFIGURATION_TYPES overridden to CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
        set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
        return()
    endif()
    list(LENGTH CMAKE_CONFIGURATION_TYPES _tmp)
    if (_tmp EQUAL 1)
        # CMAKE_CONFIGURATION_TYPES is set to one configuration - it is perfect for us. Just initialize CMAKE_BUILD_TYPE
        # for compatibility
        set(CMAKE_BUILD_TYPE ${CMAKE_CONFIGURATION_TYPES} PARENT_SCOPE)
        return()
    endif()
    if (CMAKE_DEFAULT_BUILD_TYPE)
        message(WARNING "CMAKE_CONFIGURATION_TYPES overridden to CMAKE_DEFAULT_BUILD_TYPE = ${CMAKE_DEFAULT_BUILD_TYPE}")
        set(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} PARENT_SCOPE)
        set(CMAKE_BUILD_TYPE ${CMAKE_DEFAULT_BUILD_TYPE} PARENT_SCOPE)
        return()
    endif()
    message(WARNING "CMAKE_CONFIGURATION_TYPES overridden to Debug")
    set(CMAKE_CONFIGURATION_TYPES Debug PARENT_SCOPE)
    set(CMAKE_BUILD_TYPE Debug PARENT_SCOPE)
    return()
endfunction()

# note: declaring those as a variables to support backward compatibility
# cmake_policy fires an error if an unknown policy is passed
set(CMAKE_POLICY_DEFAULT_CMP0074 NEW) # allows find_package to use packages pulled by aui.boot
set(CMAKE_POLICY_DEFAULT_CMP0135 NEW) # avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:

# fix "Failed to get the hash for HEAD" error
if(EXISTS ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
    file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/aui.boot-deps)
endif()

if (NOT AUIB_DISABLE)
    _auib_fix_multiconfiguration()
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
    # [RPATH apple]
    set(CMAKE_INSTALL_NAME_DIR "@rpath")
    set(CMAKE_INSTALL_RPATH "@loader_path/../lib")
    # [RPATH apple]
elseif(UNIX AND NOT ANDROID)
    if (CMAKE_C_COMPILER_ID MATCHES "Clang")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-rpath,$ORIGIN/../lib")
    endif()
    # [RPATH linux]
    set(CMAKE_INSTALL_RPATH $ORIGIN/../lib)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    # [RPATH linux]
endif()

define_property(GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES
        BRIEF_DOCS "Global list of aui boot root entries"
        FULL_DOCS "Global list of aui boot root entries")


define_property(GLOBAL PROPERTY AUI_BOOT_DEPS
        BRIEF_DOCS "Global list of auib_import commands"
        FULL_DOCS "Global list of auib_import commands")

# checking host system not by WIN32 because of cross compilation
if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Windows")
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_C_COMPILER_ID)
    message(FATAL_ERROR "CMAKE_C_COMPILER_ID is not set.\nnote: Please include aui.boot AFTER project() call.")
endif()

if (ANDROID_ABI)
    set(AUI_TARGET_ARCH_NAME "android-${ANDROID_ABI}")
    set(AUI_TARGET_ABI "${AUI_TARGET_ARCH_NAME}" CACHE INTERNAL "COMPILER-PROCESSOR pair")
elseif (IOS)
    set(AUI_TARGET_ARCH_NAME "ios-${CMAKE_SYSTEM_PROCESSOR}")
    set(AUI_TARGET_ABI "${AUI_TARGET_ARCH_NAME}" CACHE INTERNAL "COMPILER-PROCESSOR pair")
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
    string(TOLOWER "${CMAKE_C_COMPILER_ID}-${AUI_TARGET_ARCH_NAME}" _tmp)
    set(AUI_TARGET_ABI "${_tmp}" CACHE INTERNAL "COMPILER-PROCESSOR pair")
endif()

# checking if custom cache dir is set for the system
if(DEFINED ENV{AUIB_CACHE_DIR})
    string(REPLACE "\\" "/" _tmp $ENV{AUIB_CACHE_DIR}) # little hack to handle Windows paths
else()
    set(_tmp ${HOME_DIR}/.aui)
endif()

if(AUIB_LOCAL_CACHE)
    set(_tmp ${CMAKE_BINARY_DIR}/aui.boot)
endif()

set(AUIB_CACHE_DIR ${_tmp} CACHE PATH "Path to AUI.Boot cache" FORCE)
message(STATUS "AUI.Boot cache: ${AUIB_CACHE_DIR}")
message(STATUS "AUI.Boot target ABI: ${AUI_TARGET_ABI}")



set(AUI_BOOT_SOURCEDIR_COMPAT OFF)
if(${CMAKE_VERSION} VERSION_LESS "3.21.0")
    message(STATUS "Dependencies will be cloned to build directory, not to ${AUIB_CACHE_DIR} because you're using CMake older"
            " than 3.21. See https://github.com/aui-framework/aui/issues/6 for details.")
    set(AUI_BOOT_SOURCEDIR_COMPAT ON)
endif()


# create all required dirs
if (NOT EXISTS ${AUIB_CACHE_DIR})
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR})
endif()
if (NOT EXISTS ${AUIB_CACHE_DIR}/prefix)
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR}/prefix)
endif()
if (NOT EXISTS ${AUIB_CACHE_DIR}/repo)
    file(MAKE_DIRECTORY ${AUIB_CACHE_DIR}/repo)
endif()


function(_auib_copy_runtime_dependencies DEP_INSTALL_PREFIX)
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
            set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}" PARENT_SCOPE)
        endif()

        set(DESTINATION_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    else()
        # sometimes it's empty
        if (NOT CMAKE_LIBRARY_OUTPUT_DIRECTORY)
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
            set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}" PARENT_SCOPE)
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
                    message(STATUS "[AUI.BOOT/Runtime Dependency] ${_item} -> ${DESTINATION_DIR}/${_config}/${FILENAME}")
                endif()
                file(MAKE_DIRECTORY ${DESTINATION_DIR}/${_config})
                file(COPY ${_item} DESTINATION ${DESTINATION_DIR}/${_config})
            endforeach()
        else()
            if (NOT EXISTS ${DESTINATION_DIR}/${FILENAME})
                message(STATUS "[AUI.BOOT/Runtime Dependency] ${_item} -> ${DESTINATION_DIR}/${FILENAME}")
            endif()
            file(COPY ${_item} DESTINATION ${DESTINATION_DIR})
        endif()
    endforeach()
endfunction()

macro(_auib_update_imported_targets_list) # used for displaying imported target names
    get_property(_imported_targets_before GLOBAL PROPERTY AUIB_IMPORTED_TARGETS)
    get_property(_imported_targets_after DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    # find the new targets by excluding _imported_targets_before from _imported_targets_after
    list(REMOVE_ITEM _imported_targets_after ${_imported_targets_before})
    list(APPEND _imported_targets_before ${_imported_targets_after})
    set_property(GLOBAL PROPERTY AUIB_IMPORTED_TARGETS ${_imported_targets_before})
endmacro()

function(_auib_validate_target_installation _target _dep_install_prefix)
    if (AUIB_NO_PRECOMPILED)
        return()
    endif()
    if (AUIB_VALIDATION_LEVEL EQUAL 0)
        return()
    endif()
    option(AUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE "AUI.Boot: validate ${AUI_MODULE_NAME} installation." ON)
    if (NOT AUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE)
        return()
    endif()

    if (NOT TARGET ${_target})
        message(FATAL_ERROR "${_target} expected to be a target")
    endif()
    get_target_property(_v ${_target} TYPE)
    if (NOT _v MATCHES "_LIBRARY") # skip executables
        return()
    endif()

    get_target_property(_v ${_target} INTERFACE_AUIB_SYSTEM_LIB) # libraries defined within  auib_use_system_libs_*
    if (_v) # skip system libraries
        return()
    endif()
    set(_exclusions "${AUIB_VALID_INSTALLATION_PATHS}")

    if (AUIB_VALIDATION_LEVEL GREATER_EQUAL 2)
        list(APPEND _exclusions "${_dep_install_prefix}")
    else()
        list(APPEND _exclusions "${AUIB_CACHE_DIR}")
    endif()

    string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
    foreach(_property IMPORTED_LOCATION IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPER} IMPORTED_LOCATION_${CMAKE_BUILD_TYPE} IMPORTED_LOCATION_RELEASE IMPORTED_IMPLIB IMPORTED_OBJECTS INTERFACE_LINK_DIRECTORIES INTERFACE_INCLUDE_DIRECTORIES INTERFACE_LINK_LIBRARIES)
        get_target_property(_v ${_target} ${_property})
        if (NOT _v)
            continue()
        endif()
        foreach (_property_item ${_v})
            if (TARGET ${_property_item})
                continue()
            endif()
            if (NOT _property_item MATCHES "(.:)?[\/\\]")
                # does not contain path
                continue()
            endif()
            set(_ok FALSE)
            foreach(_match_str ${_exclusions})
                string(REPLACE "/" "\/" _match_str "^${_match_str}")
                if (_property_item MATCHES ${_match_str})
                    set(_ok TRUE)
                    break()
                endif()
            endforeach()
            if (_ok)
                continue()
            endif()
            message(FATAL_ERROR
                    "While importing ${AUI_MODULE_NAME}:\n"
                    "Imported target ${_target} depends on an out-of-tree file\n${_property_item} IN ${_property}\n"
                    "This effectively means that the library (and thus your project) is not portable. "
                    "Possible solutions:\n"
                    "1. -DAUIB_NO_PRECOMPILED=TRUE, or\n"
                    "2. -DAUIB_${AUI_MODULE_NAME_UPPER}_VALIDATE=OFF (just silences the error), or\n"
                    "3. configure ${AUI_MODULE_NAME} so it won't depend on ${_property_item}, or\n"
                    "4. if ${_property_item} is a part of another library, import that library via auib_import as well. "
                    "\n"
                    "Alternatively, you can populate AUIB_VALID_INSTALLATION_PATHS variable with valid installation path(s) "
                    "but you would probably encounter issues while deploying your app.")
        endforeach()
    endforeach ()
endfunction()

function(_auib_import_subdirectory DEP_SOURCE_DIR AUI_MODULE_NAME) # helper function to keep scope
    set(AUI_BOOT TRUE)
    add_subdirectory(${DEP_SOURCE_DIR} "aui.boot-build-${AUI_MODULE_NAME}")
endfunction()

function(_auib_precompiled_archive_name _output_var _project_name)
    if (NOT DEFINED _build_shared_libs)
        set(_build_shared_libs ${BUILD_SHARED_LIBS})
    endif()
    if (_build_shared_libs)
        set(SHARED_OR_STATIC shared)
    else()
        set(SHARED_OR_STATIC static)
    endif()
    set(_tmp "${_project_name}_${CMAKE_SYSTEM_NAME}-${AUI_TARGET_ABI}-${SHARED_OR_STATIC}-${CMAKE_BUILD_TYPE}")
    string(TOLOWER ${_tmp} _tmp)
    set(${_output_var} ${_tmp} PARENT_SCOPE)
endfunction()

macro(_auib_try_find)
    set(_mode CONFIG)
    while(TRUE)
        if (AUIB_IMPORT_COMPONENTS)
            find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET} ${_mode})
        else()
            find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET} ${_mode})
        endif()
        if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
            if (_mode STREQUAL MODULE)
                break()
            endif()
            if (AUIB_IMPORT_CONFIG_ONLY)
                break()
            else()
                set(_mode MODULE)
            endif()
        else()
            break()
        endif()
    endwhile()
    unset(_mode)
    # expose result ignoring case sensitivity
    if (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND OR ${AUI_MODULE_NAME_LOWER}_FOUND)
        set(${AUI_MODULE_NAME}_FOUND TRUE)
    else()
        set(${AUI_MODULE_NAME}_FOUND FALSE)
    endif()
endmacro()

function(_auib_try_download_precompiled_binary)
    _auib_precompiled_archive_name(_archive_name ${AUI_MODULE_NAME})
    if (AUIB_IMPORT_PRECOMPILED_URL_PREFIX)
        set(_binary_download_urls "${AUIB_IMPORT_PRECOMPILED_URL_PREFIX}/${_archive_name}.tar.gz")
        message(STATUS "Checking for precompiled package...")
    else()
        if (TAG_OR_HASH STREQUAL "latest")
            set(TAG_OR_HASH master)
        endif()
        set(_binary_download_urls "${URL}/releases/download/${TAG_OR_HASH}/${_archive_name}.tar.gz" "${URL}/releases/download/refs%2Fheads%2F${TAG_OR_HASH}/${_archive_name}.tar.gz")
        message(STATUS "GitHub detected, checking for precompiled package...")
    endif()

    foreach(_binary_download_url ${_binary_download_urls})
        if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.19)
            # since CMake 3.19 there's a way to check for file existence

            file(DOWNLOAD ${_binary_download_url} STATUS _status)
            list(GET _status 0 _status)
            if (NOT _status STREQUAL 0)
                continue()
            endif()
        endif()

        file(DOWNLOAD ${_binary_download_url} ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz SHOW_PROGRESS STATUS _status)
        list(GET _status 0 _status)
        if (NOT _status STREQUAL 0)
            continue()
        endif()

        message(STATUS "Unpacking precompiled package for ${AUI_MODULE_NAME}...")
        file(MAKE_DIRECTORY ${DEP_INSTALL_PREFIX})
        execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz
                WORKING_DIRECTORY ${DEP_INSTALL_PREFIX})

        _auib_try_find()

        file(REMOVE ${CMAKE_CURRENT_BINARY_DIR}/binary.tar.gz)

        if (${AUI_MODULE_NAME}_FOUND)
            set(_skip_compilation TRUE PARENT_SCOPE)
            file(TOUCH ${DEP_INSTALLED_FLAG})
            return()
        endif()
    endforeach()
    if (AUIB_DEBUG_PRECOMPILED STREQUAL ${AUI_MODULE_NAME})
        message(FATAL_ERROR "Precompiled binary for ${AUI_MODULE_NAME} is not available"
                "\ntrace: download urls: ${_binary_download_urls}")
    endif()
    message(STATUS "Precompiled binary for ${AUI_MODULE_NAME} is not available")
endfunction()

function(_auib_dump_with_prefix PREFIX PATH)
    file(READ "${PATH}" contents)
    STRING(REPLACE ";" "\\\\;" contents "${contents}")
    STRING(REPLACE "\n" ";" contents "${contents}")
    foreach (line ${contents})
        message("${PREFIX} ${line}")
    endforeach ()
endfunction()

function(_auib_find_git)
    auib_use_system_libs_begin()
    find_package(Git QUIET)
    set(GIT_EXECUTABLE ${GIT_EXECUTABLE} PARENT_SCOPE)
endfunction()

function(_auib_postprocess_check_hardcoded_paths _cmake_file)
    file(READ ${_cmake_file} _contents)

    string(FIND "${_contents}" "\"${AUIB_CACHE_DIR}" _match)
    if (_match STREQUAL "-1")
        return()
    endif()

    # dependency (AUI_MODULE_NAME) has another dependency (B). B is provided by auib_import (hence the aui boot
    # cache path). B's path is hardcoded which makes the package not portable.
    # this might be as a result of linking B via target_link_libraries(A ${B_LIBRARIES}) instead of
    # using CMake targets target_link_libraries(A B::B).
    # Let's find the target and fix it.
    # If we wont do this, _auib_validate_target_installation would probably yield an error.
    get_property(_previously_imported_targets GLOBAL PROPERTY AUIB_IMPORTED_TARGETS)
    foreach (_previously_imported_target ${_previously_imported_targets})
        if (NOT TARGET ${_previously_imported_target})
            continue()
        endif()
        string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE_UPPER)
        foreach(_property IMPORTED_LOCATION IMPORTED_LOCATION_${CMAKE_BUILD_TYPE_UPPER} IMPORTED_LOCATION_${CMAKE_BUILD_TYPE} IMPORTED_LOCATION_RELEASE IMPORTED_IMPLIB IMPORTED_OBJECTS INTERFACE_LINK_LIBRARIES)
            get_target_property(_v ${_previously_imported_target} ${_property})
            if (NOT _v)
                continue()
            endif()
            string(REPLACE "\"${_v}\"" ${_previously_imported_target} _contents2 "${_contents}")
            if (_contents2 STREQUAL "${_contents}")
                continue()
            endif()
            message(STATUS "[AUI.BOOT] Fix: in \"${_cmake_file}\": \"${_v}\" -> ${_previously_imported_target}")
            set(_contents "${_contents2}")
        endforeach()
    endforeach ()
    file(WRITE ${_cmake_file} "${_contents}")
endfunction()

function(_auib_postprocess)
    # tries to fix obvious CMake violations, if any.
    option(AUIB_${AUI_MODULE_NAME_UPPER}_POSTPROCESS "AUI.Boot: apply fixes on ${AUI_MODULE_NAME} installation if needed." ON)
    if (NOT AUIB_${AUI_MODULE_NAME_UPPER}_POSTPROCESS)
        return()
    endif()
    file(GLOB_RECURSE _cmakes ${DEP_INSTALL_PREFIX}/*.cmake)
    foreach(_cmake ${_cmakes})
        _auib_postprocess_check_hardcoded_paths(${_cmake})
    endforeach ()

endfunction()

# TODO add a way to provide file access to the repository
function(auib_import AUI_MODULE_NAME URL)
    list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
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
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)

    set(options ADD_SUBDIRECTORY ARCHIVE CONFIG_ONLY IMPORTED_FROM_CONFIG)
    set(oneValueArgs VERSION CMAKE_WORKING_DIR PRECOMPILED_URL_PREFIX LINK)

    set(multiValueArgs CMAKE_ARGS COMPONENTS REQUIRES)
    cmake_parse_arguments(AUIB_IMPORT "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )


    # check for dependencies
    foreach (_dep ${AUIB_IMPORT_REQUIRES})
        set(_dep_root_var ${_dep}_ROOT)
        if (NOT DEFINED ${_dep_root_var})
            message(FATAL_ERROR "${AUI_MODULE_NAME} requires ${_dep}, but it's not available (${_dep_root_var} is not set)")
        endif()
        set(_dep_root_value ${${_dep_root_var}})
        set(_dep_root_value_installed ${_dep_root_value}/INSTALLED)
        if (NOT EXISTS ${_dep_root_value_installed})
            message(FATAL_ERROR "${AUI_MODULE_NAME} requires ${_dep}, but it's not available (${_dep_root_value_installed} does not exist)")
        endif()

        # add it to AUI_BOOT_ROOT_ENTRIES if needed
        get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
        list(JOIN AUI_BOOT_ROOT_ENTRIES , AUI_BOOT_ROOT_ENTRIES)
        if (NOT "${AUI_BOOT_ROOT_ENTRIES}" MATCHES ".*${_dep_root_var}.*")
            set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${_dep_root_var}=${_dep_root_value}")
        endif ()
    endforeach()

    if (AUIB_IMPORT_ARCHIVE AND AUIB_IMPORT_VERSION)
        message(FATAL_ERROR "ARCHIVE and VERSION arguments are incompatible")
    endif()

    option(AUIB_${AUI_MODULE_NAME_UPPER}_AS "AUI.Boot: import ${AUI_MODULE_NAME} as a subdirectory.")
    if (AUIB_${AUI_MODULE_NAME_UPPER}_AS OR AUIB_ALL_AS OR AUIB_IMPORT_ADD_SUBDIRECTORY)
        set(DEP_ADD_SUBDIRECTORY TRUE)
    else()
        set(DEP_ADD_SUBDIRECTORY FALSE)
    endif()

    set(AUI_MODULE_PREFIX ${AUI_MODULE_NAME_LOWER})

    set(TAG_OR_HASH latest)
    if (AUIB_IMPORT_ARCHIVE)
        string(SHA1 TAG_OR_HASH ${URL})
        set(TAG_OR_HASH "${TAG_OR_HASH}, autogenerated from url hash")
    elseif (AUIB_IMPORT_HASH)
        set(TAG_OR_HASH ${AUIB_IMPORT_HASH})
    elseif(AUIB_IMPORT_VERSION)
        set(TAG_OR_HASH ${AUIB_IMPORT_VERSION})
    endif()

    # should restrict version length; in order to make equal "f116a123b9b44f362c96632ad5cec980aab8b46c" and "f116a123"
    if (NOT AUIB_IMPORT_ARCHIVE)
        string(LENGTH ${TAG_OR_HASH} _length)
        if (_length GREATER 16)
            string(SUBSTRING ${TAG_OR_HASH} 0 7 TAG_OR_HASH)
        endif()
    endif()

    if (NOT CMAKE_BUILD_TYPE)
        set(CMAKE_BUILD_TYPE Debug)
    endif()

    if (NOT DEFINED BUILD_SHARED_LIBS)
        # default it to ON
        set(BUILD_SHARED_LIBS ON)
    endif()

    # process LINK argument
    if (AUIB_IMPORT_LINK)
        if (AUIB_IMPORT_LINK STREQUAL "STATIC")
            set(_build_shared_libs FALSE)
        elseif (AUIB_IMPORT_LINK STREQUAL "SHARED")
            set(_build_shared_libs TRUE)
        else()
            message(FATAL_ERROR "invalid value \"${AUIB_IMPORT_LINK}\" for LINK argument")
        endif()
    else()
        set(_build_shared_libs ${BUILD_SHARED_LIBS})
    endif()


    if (_build_shared_libs)
        set(SHARED_OR_STATIC shared)
    else()
        set(SHARED_OR_STATIC static)
        set(CMAKE_POSITION_INDEPENDENT_CODE ON) # -fPIC required on linux
    endif()

    # [[BUILD_SPECIFIER]]
    set(BUILD_SPECIFIER "${TAG_OR_HASH}/${AUI_TARGET_ABI}-${CMAKE_BUILD_TYPE}-${SHARED_OR_STATIC}/${CMAKE_GENERATOR}/${AUIB_IMPORT_CMAKE_ARGS}")
    string(REPLACE ";" " " BUILD_SPECIFIER "${BUILD_SPECIFIER}")

    # convert BUILD_SPECIFIER to hash; on windows msvc path length restricted by 260 chars
    string(MD5 BUILD_SPECIFIER ${BUILD_SPECIFIER})
    # [[BUILD_SPECIFIER]]

    # append module name to build specifier in order to distinguish modules in prefix/ dir
    set(BUILD_SPECIFIER "${AUI_MODULE_NAME_LOWER}/${BUILD_SPECIFIER}")

    set(DEP_INSTALL_PREFIX "${AUIB_CACHE_DIR}/prefix/${BUILD_SPECIFIER}")

    if (AUIB_IMPORT_PRECOMPILED_URL_PREFIX)
        if (EXISTS ${AUIB_IMPORT_PRECOMPILED_URL_PREFIX})
            # local file
            set(DEP_INSTALL_PREFIX ${AUIB_IMPORT_PRECOMPILED_URL_PREFIX})
        endif()
    endif()


    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()
    endif()

    if (DEP_ADD_SUBDIRECTORY)
        set(DEP_AS_DIR "${AUIB_CACHE_DIR}/repo/${AUI_MODULE_PREFIX}/as/${TAG_OR_HASH}")

        # the AUI_MODULE_NAME is used to hint IDEs (i.e. CLion) about actual project name
        set(DEP_SOURCE_DIR "${DEP_AS_DIR}/${AUI_MODULE_NAME_LOWER}")
        set(DEP_BINARY_DIR "${DEP_AS_DIR}/build/${BUILD_SPECIFIER}")
        set(DEP_FETCHED_FLAG ${DEP_AS_DIR}/FETCHED)
    else()
        set(DEP_SOURCE_DIR "${AUIB_CACHE_DIR}/repo/${AUI_MODULE_PREFIX}/src")
        set(DEP_BINARY_DIR "${AUIB_CACHE_DIR}/repo/${AUI_MODULE_PREFIX}/build/${BUILD_SPECIFIER}")
        set(DEP_FETCHED_FLAG ${DEP_SOURCE_DIR}/FETCHED)
    endif()

    # invalidate all previous values.
    foreach(_v2 FOUND
            INCLUDE_DIR
            LIBRARY
            LIBRARY_DEBUG
            LIBRARY_RELEASE
            ROOT
            ROOT_DIR # OPENSSL_ROOT_DIR
            DIR)
        foreach(_v1 ${AUI_MODULE_NAME} ${AUI_MODULE_NAME_UPPER})
            unset(${_v1}_${_v2} PARENT_SCOPE)
            unset(${_v1}_${_v2} CACHE)
        endforeach()
    endforeach()

    # variable is CACHE here in order to make it as global as possible.
    set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")
    set(${AUI_MODULE_NAME}_ROOT_DIR ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    # creating uppercase variables in order to ease the case insensitive checks
    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_INSTALL_PREFIX} PARENT_SCOPE)
    set(${AUI_MODULE_NAME_UPPER}_ROOT_DIR ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")

    set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)

    # TODO add protocol check
    if(AUI_BOOT_SOURCEDIR_COMPAT)
        unset(SOURCE_BINARY_DIRS_ARG)
    else()
        if (NOT AUI_BOOT AND NOT AUIB_SKIP_REPOSITORY_WAIT AND NOT AUIB_IMPORT_IMPORTED_FROM_CONFIG) # recursive deadlock fix
            if (NOT _locked)
                set(_locked TRUE)
                file(LOCK "${AUIB_CACHE_DIR}/repo.lock" RESULT_VARIABLE _error TIMEOUT 1) # try lock without the message
                if (_error)
                    message(STATUS "Waiting for repository... (simultaneous configure processes may break something!)")
                    file(LOCK "${AUIB_CACHE_DIR}/repo.lock")
                endif()
            endif()
        endif()
        set(SOURCE_BINARY_DIRS_ARG SOURCE_DIR ${DEP_SOURCE_DIR}
                BINARY_DIR ${DEP_BINARY_DIR})
    endif()

    if (NOT DEP_ADD_SUBDIRECTORY)
        # avoid compilation if we have existing installation
        if (EXISTS ${DEP_INSTALLED_FLAG})
            _auib_try_find()
        endif()
    endif()

    if ((NOT EXISTS ${DEP_INSTALLED_FLAG} OR NOT ${AUI_MODULE_NAME}_FOUND AND NOT DEP_ADD_SUBDIRECTORY) OR ((NOT EXISTS ${DEP_FETCHED_FLAG}) AND DEP_ADD_SUBDIRECTORY))
        # some shit with INSTALLED flag because find_package finds by ${AUI_MODULE_NAME}_ROOT only if REQUIRED flag is set
        # so we have to compile and install
        if (NOT DEP_ADD_SUBDIRECTORY)
            if (NOT EXISTS ${DEP_INSTALLED_FLAG})
                message(STATUS "${AUI_MODULE_NAME}: resolving because ${DEP_INSTALLED_FLAG} does not exist")
            else()
                message(STATUS "${AUI_MODULE_NAME}: resolving because find_package could not find package in ${DEP_INSTALL_PREFIX}")
            endif()
        else()
            if (EXISTS ${DEP_SOURCE_DIR})
                file(REMOVE_RECURSE ${DEP_SOURCE_DIR})
            endif()
        endif()

        set(${AUI_MODULE_NAME}_FOUND FALSE) # reset the FOUND flag; in some cases it may have been TRUE here

        unset(_skip_compilation) # set by _auib_try_download_precompiled_binary

        message(STATUS "Fetching ${AUI_MODULE_NAME} (${TAG_OR_HASH})")
        # check for GitHub Release
        if ((URL MATCHES "^https://github.com/" OR AUIB_IMPORT_PRECOMPILED_URL_PREFIX) AND NOT AUIB_NO_PRECOMPILED AND NOT DEP_ADD_SUBDIRECTORY)
            _auib_try_download_precompiled_binary()
        endif()

        if (NOT _skip_compilation)
            if (AUIB_FORCE_PRECOMPILED)
                message(FATAL_ERROR "Can't find a precompiled package for ${AUI_MODULE_NAME}. (-DAUIB_FORCE_PRECOMPILED)")
            endif()
            set(_skip_fetch FALSE)
            if (EXISTS "${DEP_SOURCE_DIR}/.git")
                # let's try to checkout a local repository
                _auib_find_git()
                if(GIT_EXECUTABLE)
                    execute_process(COMMAND ${GIT_EXECUTABLE} checkout -f ${AUIB_IMPORT_VERSION}
                            WORKING_DIRECTORY ${DEP_SOURCE_DIR}
                            RESULT_VARIABLE _errored
                            OUTPUT_QUIET
                            ERROR_QUIET)
                    if (NOT _errored)
                        message(STATUS "${DEP_SOURCE_DIR}: checkout'ed ${AUIB_IMPORT_VERSION}")
                        set(_skip_fetch TRUE)
                    endif()
                endif()
            endif()
            if (NOT _skip_fetch)
                file(REMOVE_RECURSE ${DEP_SOURCE_DIR})
            endif()
            file(REMOVE_RECURSE ${DEP_BINARY_DIR})
        endif()

        # check for local existence
        if (EXISTS ${URL})
            get_filename_component(DEP_SOURCE_DIR ${URL} ABSOLUTE)
            message(STATUS "Using local: ${DEP_SOURCE_DIR}")
        else()
            if (NOT _skip_compilation)

                include(FetchContent)

                if (AUIB_IMPORT_ARCHIVE)
                    set(_import_type URL)
                else()
                    set(_import_type GIT_REPOSITORY)
                endif()
                if (NOT _skip_fetch)
                    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.30.0)
                        # deprecated "FetchContent_Populate(${AUI_MODULE_NAME}_FC)", using a "new" form instead
                        FetchContent_Populate(${AUI_MODULE_NAME}_FC
                                PREFIX "${CMAKE_BINARY_DIR}/aui.boot-deps/${AUI_MODULE_NAME}"
                                ${_import_type} "${URL}"
                                GIT_TAG ${AUIB_IMPORT_VERSION}
                                GIT_PROGRESS TRUE # show progress of download
                                USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                                USES_TERMINAL_UPDATE TRUE # show progress in ninja generator
                                ${SOURCE_BINARY_DIRS_ARG}
                        )
                    else()
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
                    endif()

                    FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                            BINARY_DIR DEP_BINARY_DIR
                            SOURCE_DIR DEP_SOURCE_DIR
                    )
                    message(STATUS "Fetched ${AUI_MODULE_NAME} to ${DEP_SOURCE_DIR}")
                    if (NOT AUI_BOOT_SOURCEDIR_COMPAT)
                        file(TOUCH ${DEP_FETCHED_FLAG})
                    endif()
                endif ()
            endif()
        endif()

        if (NOT _skip_compilation)
            if (AUIB_IMPORT_CMAKE_WORKING_DIR)
                set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
            endif()

            if (NOT DEP_ADD_SUBDIRECTORY)
                message(STATUS "Compiling ${AUI_MODULE_NAME}")

                get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
                unset(FORWARDED_LIBS)
                foreach (_entry ${AUI_BOOT_ROOT_ENTRIES})
                    list(APPEND FORWARDED_LIBS "-D${_entry}")
                endforeach()
                set(FINAL_CMAKE_ARGS
                        -DAUI_BOOT=TRUE
                        --no-warn-unused-cli # zaebalo
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

                unset(_forwardable_vars)
                get_property(_forwardable_vars GLOBAL PROPERTY AUIB_FORWARDABLE_VARS)
                if(ANDROID)
                    list(APPEND _forwardable_vars
                            CMAKE_EXPORT_COMPILE_COMMANDS
                            ANDROID_PLATFORM
                            ANDROID_ABI
                            CMAKE_ANDROID_ARCH_ABI
                            ANDROID_NDK
                            CMAKE_ANDROID_NDK
                    )
                endif()

                get_cmake_property(CACHE_VARS CACHE_VARIABLES)

                foreach(CACHE_VAR ${CACHE_VARS})
                    if(_forwardable)
                        list(APPEND _forwardable_vars ${CACHE_VAR})
                    endif()
                endforeach()

 
                # force msvc compiler to parallel build
                if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")                           # MSVC but exclude clang-cl
                    set_property(DIRECTORY APPEND PROPERTY COMPILE_OPTIONS "-MP")   # Parallel compilation
                endif()

                if (IOS)
                    # fix multiple definitions on ios
                    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-error-implicit-function-declaration")
                endif()

                # forward all necessary variables to child cmake build
                foreach(_varname
                        AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT
                        AUIB_NO_PRECOMPILED
                        AUIB_FORCE_PRECOMPILED
                        AUIB_TRACE_BUILD_SYSTEM
                        AUIB_SKIP_REPOSITORY_WAIT
                        AUIB_CACHE_DIR
                        CMAKE_C_FLAGS
                        CMAKE_CXX_FLAGS
                        CMAKE_GENERATOR_PLATFORM
                        CMAKE_VS_PLATFORM_NAME
                        CMAKE_BUILD_TYPE
                        CMAKE_CONFIGURATION_TYPES
                        CMAKE_CROSSCOMPILING
                        CMAKE_CROSSCOMPILING_EMULATOR
                        CMAKE_MACOSX_RPATH
                        CMAKE_SYSTEM_NAME
                        CMAKE_SYSTEM_VERSION
                        CMAKE_SYSTEM_PROCESSOR
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
                        CMAKE_OSX_DEPLOYMENT_TARGET
                        DEPLOYMENT_TARGET
                        CMAKE_POLICY_DEFAULT_CMP0074 # find_package uses *_ROOT variables
                        PLATFORM
                        CMAKE_OSX_ARCHITECTURES
                        XCODE_VERSION
                        SDK_VERSION
                        APPLE_TARGET_TRIPLE
                        AUI_IOS_CODE_SIGNING_REQUIRED
                        CMAKE_POSITION_INDEPENDENT_CODE
                        ${_forwardable_vars})

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
                list(APPEND FINAL_CMAKE_ARGS "-DBUILD_SHARED_LIBS=${_build_shared_libs}")

                file(MAKE_DIRECTORY ${DEP_INSTALL_PREFIX})
                file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                message("Configuring CMake ${AUI_MODULE_NAME}:${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}")
                execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/configure.log
                )
                _auib_dump_with_prefix("[Configuring ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/configure.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(STATUS "Dependency CMake configure failed, clearing dir and trying again...")
                    file(REMOVE_RECURSE ${DEP_BINARY_DIR})
                    file(MAKE_DIRECTORY ${DEP_BINARY_DIR})
                    execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                            WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                            RESULT_VARIABLE STATUS_CODE
                            OUTPUT_FILE ${DEP_INSTALL_PREFIX}/configure.log
                    )
                    _auib_dump_with_prefix("[Configuring ${AUI_MODULE_NAME} (2)]" ${DEP_INSTALL_PREFIX}/configure.log)
                    if (NOT STATUS_CODE EQUAL 0)
                        message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
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
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/build.log
                )
                _auib_dump_with_prefix("[Building ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/build.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(FATAL_ERROR "Dependency build failed: ${AUI_MODULE_NAME}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
                endif()

                message(STATUS "Installing ${AUI_MODULE_NAME}")
                execute_process(COMMAND
                        ${CMAKE_COMMAND}
                        --install .
                        --config ${CMAKE_BUILD_TYPE} # fix vs and xcode generators

                        WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                        RESULT_VARIABLE STATUS_CODE
                        OUTPUT_FILE ${DEP_INSTALL_PREFIX}/install.log
                        OUTPUT_QUIET)
                _auib_dump_with_prefix("[Installing ${AUI_MODULE_NAME}]" ${DEP_INSTALL_PREFIX}/install.log)

                if (NOT STATUS_CODE EQUAL 0)
                    message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}\nnote: check logs in ${DEP_INSTALL_PREFIX}")
                endif()
                if (NOT EXISTS ${DEP_INSTALL_PREFIX})
                    message(FATAL_ERROR "Dependency failed to install: ${AUI_MODULE_NAME}\nnote: check build logs in ${DEP_INSTALL_PREFIX}")
                endif()
                _auib_postprocess()
                file(TOUCH ${DEP_INSTALLED_FLAG})

                message(STATUS "Cleaning up build directory")
                file(REMOVE_RECURSE ${DEP_BINARY_DIR})

            endif()
        endif()
    else()
        if (DEP_ADD_SUBDIRECTORY AND AUIB_IMPORT_CMAKE_WORKING_DIR)
            set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
        endif()
    endif()
    if (_locked)
        set(_locked FALSE)
        file(LOCK "${AUIB_CACHE_DIR}/repo.lock" RELEASE)
    endif()
    if (DEP_ADD_SUBDIRECTORY)
        set(${AUI_MODULE_NAME}_ROOT ${DEP_SOURCE_DIR})
        _auib_import_subdirectory(${DEP_SOURCE_DIR} ${AUI_MODULE_NAME})
        message(STATUS "${AUI_MODULE_NAME} imported as a subdirectory: ${DEP_SOURCE_DIR}")
    elseif(NOT ${AUI_MODULE_NAME}_FOUND)
        _auib_try_find()

        if (NOT ${AUI_MODULE_NAME}_FOUND)
            # print verbosely find procedure

            set(CMAKE_FIND_DEBUG_MODE TRUE)

            # a slight modified _auib_try_find macro to verbosely print debug info
            set(_mode CONFIG)
            message("[AUI.BOOT] Verbose output:")
            while(TRUE)
                if (AUIB_IMPORT_COMPONENTS)
                    find_package(${AUI_MODULE_NAME} COMPONENTS ${AUIB_IMPORT_COMPONENTS} ${FINDPACKAGE_QUIET} ${_mode})
                else()
                    find_package(${AUI_MODULE_NAME} ${FINDPACKAGE_QUIET} ${_mode})
                endif()
                if (NOT (${AUI_MODULE_NAME}_FOUND OR ${AUI_MODULE_NAME_UPPER}_FOUND))
                    if (_mode STREQUAL MODULE)
                        message("[AUI.BOOT] Dependency not found - giving up")
                        break()
                    endif()
                    if (AUIB_IMPORT_CONFIG_ONLY)
                        break()
                    else()
                        set(_mode MODULE)
                    endif()
                    message("[AUI.BOOT] Using config instead")
                else()
                    break()
                endif()
            endwhile()
            unset(_mode)

            # list possible find_package names if available
            file(GLOB_RECURSE _find "${DEP_INSTALL_PREFIX}/*onfig.cmake")
            unset(possible_names)
            foreach(_i ${_find})
                get_filename_component(_name ${_i} NAME)
                string(REGEX REPLACE "-?([Cc]onfig)\\.cmake" "" _name ${_name})
                list(APPEND possible_names "\"${_name}\"")
            endforeach()

            # construct error message
            set(error_message "AUI.Boot could not resolve dependency: ${AUI_MODULE_NAME}\nnote: check build logs in ${DEP_INSTALL_PREFIX}")
            set(error_message "${error_message}\nnote: package names are case sensitive")
            if (possible_names)
                string(JOIN " or " possible_names_joined ${possible_names})
                set(error_message "${error_message}\nnote: did you mean " ${possible_names_joined} ?)
            else()
                set(error_message "${error_message}\nnote: looks like a config file does not exist for your project (${AUI_MODULE_NAME}Config.cmake or ${AUI_MODULE_NAME}-config.cmake).")
            endif()
            message(FATAL_ERROR ${error_message})
        endif()
    endif()

    _auib_copy_runtime_dependencies(${DEP_INSTALL_PREFIX})

    if (NOT DEP_ADD_SUBDIRECTORY)
        set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_ROOT=${${AUI_MODULE_NAME}_ROOT}")
        set_property(GLOBAL APPEND PROPERTY AUI_BOOT_ROOT_ENTRIES "${AUI_MODULE_NAME}_DIR=${${AUI_MODULE_NAME}_DIR}")
    endif()

    set_property(GLOBAL APPEND PROPERTY AUI_BOOT_IMPORTED_MODULES ${AUI_MODULE_NAME_LOWER})

    # display the imported targets (available since CMake 3.21)
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.21)
        _auib_update_imported_targets_list()

        set(_imported_target_pretty "")
        foreach (_target ${_imported_targets_after})
            get_target_property(_is_sys ${_target} INTERFACE_AUIB_SYSTEM_LIB)
            if (_is_sys)
                list(APPEND _imported_target_pretty "sys ${_target}")
            else()
                list(APPEND _imported_target_pretty "${_target}")
            endif()
        endforeach()

        message(STATUS "Imported: ${AUI_MODULE_NAME} (${_imported_target_pretty}) (${${AUI_MODULE_NAME}_ROOT}) (version ${TAG_OR_HASH})")
        foreach (_target ${_imported_targets_after})
            _auib_validate_target_installation(${_target} ${DEP_INSTALL_PREFIX})
        endforeach()
    else()
        message(STATUS "Imported: ${AUI_MODULE_NAME} (${${AUI_MODULE_NAME}_ROOT}) (version ${TAG_OR_HASH})")
    endif()

    # save arguments for later use by dependent modules
    if (NOT AUIB_IMPORT_IMPORTED_FROM_CONFIG)
        string(REPLACE ";" " " _forwarded_import_args "${ARGV}")
        set(_precompiled_url "")
        if (EXISTS ${DEP_INSTALL_PREFIX})
            if (AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
                set(_precompiled_url " PRECOMPILED_URL_PREFIX \${CMAKE_CURRENT_LIST_DIR}/deps/${BUILD_SPECIFIER}")
                install(DIRECTORY ${DEP_INSTALL_PREFIX} DESTINATION "deps/${AUI_MODULE_NAME_LOWER}")
            endif()
        endif()
        set_property(GLOBAL APPEND_STRING PROPERTY AUI_BOOT_DEPS "auib_import(${_forwarded_import_args} IMPORTED_FROM_CONFIG ${_precompiled_url})\n")
    endif()
    _auib_find_git()
    if (GIT_EXECUTABLE AND NOT AUIB_IMPORT_ARCHIVE)
        execute_process(COMMAND ${GIT_EXECUTABLE} status
                WORKING_DIRECTORY ${DEP_SOURCE_DIR}
                OUTPUT_VARIABLE git_status
        )
        if(NOT git_status MATCHES "HEAD")
            message(WARNING "${AUIB_IMPORT_NAME} You are staying on a branch or did not specify the version control, please specify a tag or hash VERSION!\nSee https://aui-framework.github.io/develop/md_docs_2AUI_01Boot.html#version")
        endif ()
    endif ()
endfunction()


macro(auib_use_system_libs_begin)
    get_property(_imported_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)
    set_property(GLOBAL PROPERTY AUIB_SYSTEM_LIBS_BEGIN ${_imported_targets})

    set(AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH TRUE)
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH TRUE)
endmacro()

macro(auib_use_system_libs_end)
    get_property(_imported_targets_before GLOBAL PROPERTY AUIB_SYSTEM_LIBS_BEGIN)
    get_property(_imported_targets_after DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY IMPORTED_TARGETS)

    list(LENGTH _imported_targets_before _n)
    if (NOT _n STREQUAL 0)
        # find the new targets by excluding _imported_targets_before from _imported_targets_after
        list(REMOVE_ITEM _imported_targets_after ${_imported_targets_before})

        foreach (_t ${_imported_targets_after})
            set_target_properties(${_t} PROPERTIES INTERFACE_AUIB_SYSTEM_LIB ON)
        endforeach()
    endif()

    set(CMAKE_FIND_USE_CMAKE_SYSTEM_PATH ${AUIB_PREV_CMAKE_FIND_USE_CMAKE_SYSTEM_PATH})
    set(CMAKE_FIND_USE_SYSTEM_ENVIRONMENT_PATH FALSE)
endmacro()

macro(auib_precompiled_binary)
    set(CPACK_GENERATOR "TGZ")
    get_property(_auib_deps GLOBAL PROPERTY AUI_BOOT_DEPS)
    set(AUIB_DEPS ${_auib_deps})
    if (AUIB_PRODUCED_PACKAGES_SELF_SUFFICIENT)
        foreach (_location ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
            set(_location "${_location}/aui.boot.cmake")
            if (EXISTS ${_location})
                install(FILES ${_location} DESTINATION ".")
                set(_AUIB_DEPS [[
include(${CMAKE_CURRENT_LIST_DIR}/aui.boot.cmake)
get_filename_component(_aui_boot_current_list_file "${CMAKE_CURRENT_LIST_FILE}" PATH)
_auib_copy_runtime_dependencies(${_aui_boot_current_list_file})

]])
                set(AUIB_DEPS "${_AUIB_DEPS}${AUIB_DEPS}")
                break()
            endif()
        endforeach()
        set(AUIB_DEPS "list(APPEND AUIB_VALID_INSTALLATION_PATHS \${CMAKE_CURRENT_LIST_DIR})\n${AUIB_DEPS}")
    endif()
    _auib_precompiled_archive_name(CPACK_PACKAGE_FILE_NAME ${PROJECT_NAME})
    message(STATUS "[AUI.BOOT] Output precompiled archive name: ${CPACK_PACKAGE_FILE_NAME}")
    set(CPACK_VERBATIM_VARIABLES YES)
    set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
    include(CPack)
endmacro()
