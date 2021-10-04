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

set(CMAKE_POLICY_DEFAULT_CMP0074 NEW)

if(WIN32 AND NOT CYGWIN)
    set(HOME_DIR $ENV{USERPROFILE})
else()
    set(HOME_DIR $ENV{HOME})
endif()

if (NOT CMAKE_CXX_COMPILER_ID)
    message(FATAL_ERROR "Please include aui.boot AFTER project() call.")
endif()

if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(AUI_TARGET_PROCESSOR_NAME "x86_64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(AUI_TARGET_PROCESSOR_NAME "x86")
    endif()
else ()
    set(AUI_TARGET_PROCESSOR_NAME ${CMAKE_SYSTEM_PROCESSOR})
endif ()


set(AUI_CACHE_DIR ${HOME_DIR}/.aui CACHE PATH "Path to AUI.Boot cache")
string(TOLOWER "${CMAKE_CXX_COMPILER_ID}-${AUI_TARGET_PROCESSOR_NAME}" _tmp)
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
    set(oneValueArgs VERSION CMAKE_WORKING_DIR COMPONENTS)
    set(multiValueArgs CMAKE_ARGS)
    cmake_parse_arguments(AUIB_IMPORT "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )
    set(TAG_OR_HASH latest)
    if (AUIB_IMPORT_HASH)
        set(TAG_OR_HASH ${AUIB_IMPORT_HASH})
    elseif(AUIB_IMPORT_VERSION)
        set(TAG_OR_HASH ${AUIB_IMPORT_VERSION})
    endif()
    set(DEP_INSTALL_PREFIX "${AUI_CACHE_DIR}/prefix/${AUI_MODULE_NAME}/${TAG_OR_HASH}/${AUI_TARGET_ABI}/${CMAKE_BUILD_TYPE}")

    # append our location to module path
    #if (NOT "${DEP_INSTALL_PREFIX}" IN_LIST CMAKE_PREFIX_PATH)
    #    list(APPEND CMAKE_PREFIX_PATH ${DEP_INSTALL_PREFIX})
    #endif()

    set(${AUI_MODULE_NAME}_DIR ${DEP_INSTALL_PREFIX})

    string(REGEX REPLACE "[a-z]+:\\/\\/" "" URL_PATH ${URL})
    set(DEP_SOURCE_DIR "${AUI_CACHE_DIR}/repo/${URL_PATH}")
    if (NOT ${AUI_MODULE_NAME}_ROOT
            OR NOT EXISTS ${${AUI_MODULE_NAME}_ROOT}
            )
        # avoid compilation if we have existing installation
        set(DEP_INSTALLED_FLAG ${DEP_INSTALL_PREFIX}/INSTALLED)

        # some shit with INSTALLED flag because find_package finds by ${AUI_MODULE_NAME}_ROOT only if REQUIRED flag is set
        if (NOT EXISTS ${DEP_INSTALLED_FLAG})
            # so we have to compile and install

            include(FetchContent)
            # TODO add protocol check
            message(STATUS "Fetching ${AUI_MODULE_NAME}")
            file(LOCK "${AUI_CACHE_DIR}/repo.lock")
            FetchContent_Declare(${AUI_MODULE_NAME}_FC
                    GIT_REPOSITORY "${URL}"
                    GIT_TAG ${AUIB_IMPORT_VERSION}
                    GIT_PROGRESS TRUE # show progress of download
                    USES_TERMINAL_DOWNLOAD TRUE # show progress in ninja generator
                    SOURCE_DIR ${DEP_SOURCE_DIR}
                    )

            FetchContent_Populate(${AUI_MODULE_NAME}_FC)


            FetchContent_GetProperties(${AUI_MODULE_NAME}_FC
                    BINARY_DIR DEP_BINARY_DIR
                    #SOURCE_DIR DEP_SOURCE_DIR
                    )

            message(STATUS "Compiling ${AUI_MODULE_NAME}")
            if (AUIB_IMPORT_CMAKE_WORKING_DIR)
                set(DEP_SOURCE_DIR "${DEP_SOURCE_DIR}/${AUIB_IMPORT_CMAKE_WORKING_DIR}")
            endif()
            set(FINAL_CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                                 -DAUI_BOOT=TRUE
                                 ${AUIB_IMPORT_CMAKE_ARGS}
                                 -DCMAKE_INSTALL_PREFIX:PATH=${DEP_INSTALL_PREFIX}
                                 -G "${CMAKE_GENERATOR}")

            if (AUIB_IMPORT_COMPONENTS)
                set(FINAL_CMAKE_ARGS -DCMAKE_SKIP_INSTALL_ALL_DEPENDENCY=TRUE )
            endif()

            execute_process(COMMAND ${CMAKE_COMMAND} ${DEP_SOURCE_DIR} ${FINAL_CMAKE_ARGS}
                    WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                    RESULT_VARIABLE STATUS_CODE)

            if (NOT STATUS_CODE EQUAL 0)
                message(FATAL_ERROR "CMake configure failed: ${STATUS_CODE}")
            endif()



            message(STATUS "Installing ${AUI_MODULE_NAME}")
            execute_process(COMMAND ${CMAKE_COMMAND} --build ${DEP_BINARY_DIR} --target ${AUIB_IMPORT_COMPONENTS} install
                    WORKING_DIRECTORY "${DEP_BINARY_DIR}"
                    RESULT_VARIABLE ERROR_CODE)

            if (NOT STATUS_CODE EQUAL 0)
                message(FATAL_ERROR "CMake build failed: ${STATUS_CODE}")
            endif()
            file(TOUCH ${DEP_INSTALLED_FLAG})
            file(LOCK "${AUI_CACHE_DIR}/repo.lock" RELEASE)
        endif()
    endif()
    set(${AUI_MODULE_NAME}_ROOT ${DEP_INSTALL_PREFIX} CACHE FILEPATH "Path to ${AUI_MODULE_NAME} provided by AUI.Boot.")
    find_package(${AUI_MODULE_NAME} REQUIRED)

    # create links to runtime dependencies
    if (WIN32)
        set(LIB_EXT dll)
    elseif(APPLE)
        set(LIB_EXT dylib)
    else()
        set(LIB_EXT so)
    endif()
    file(GLOB DEP_RUNTIME LIST_DIRECTORIES false ${DEP_INSTALL_PREFIX}/bin/*.${LIB_EXT})
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    foreach(_item ${DEP_RUNTIME})
        get_filename_component(FILENAME ${_item} NAME)
        if (CMAKE_CONFIGURATION_TYPES)
            foreach(_config ${CMAKE_CONFIGURATION_TYPES})
                set(_copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_config}/${FILENAME})
                if (NOT EXISTS ${_copy})
                    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${_config})
                    file(CREATE_LINK ${_item} ${_copy})
                endif()
            endforeach()
        else()
            set(_copy ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${FILENAME})
            if (NOT EXISTS ${_copy})
                file(CREATE_LINK ${_item} ${_copy})
            endif()
        endif()
    endforeach()
endmacro()