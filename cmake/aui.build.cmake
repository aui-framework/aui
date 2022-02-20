# ======================================================================================================================
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
# ======================================================================================================================

# CMake AUI building functions

define_property(GLOBAL PROPERTY TESTS_SRCS
        BRIEF_DOCS "Global list of test files"
        FULL_DOCS "Global list of test files")
define_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS
        BRIEF_DOCS "Global list of test include dirs"
        FULL_DOCS "Global list of test include dirs")
define_property(GLOBAL PROPERTY TESTS_DEPS
        BRIEF_DOCS "Global list of test dependencies"
        FULL_DOCS "Global list of test dependencies")

define_property(TARGET PROPERTY INTERFACE_AUI_WHOLEARCHIVE
        BRIEF_DOCS "Use wholearchive when linking this library to another"
        FULL_DOCS "Use wholearchive when linking this library to another")

set_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS "")
set_property(GLOBAL PROPERTY TESTS_SRCS "")

# generator expressions for install(CODE [[ ... ]])
set(CMAKE_POLICY_DEFAULT_CMP0087 NEW)
set(AUI_BUILD_PREVIEW OFF CACHE BOOL "Enable aui.preview plugin target")
set(AUI_INSTALL_RUNTIME_DEPENDENCIES ${AUI_BOOT} CACHE BOOL "Install runtime dependencies along with the project")

cmake_policy(SET CMP0072 NEW)

if (ANDROID OR IOS)
    set(_build_shared OFF)
    message(STATUS "Forcing static build because you are building for mobile platform.")
else()
    set(_build_shared ON)
endif()
set(BUILD_SHARED_LIBS ${_build_shared} CACHE BOOL "Build using shared libraries")

# platform definitions
# platform exclusion (AUI/Platform/<platform name>/...)
set(AUI_EXCLUDE_PLATFORMS android linux macos win32 ios)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(AUI_DEBUG TRUE)
else()
    set(AUI_DEBUG TRUE)
endif()

if (WIN32)
    set(AUI_PLATFORM_WIN 1 CACHE BOOL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS win32)
else()
    set(AUI_PLATFORM_WIN 0 CACHE BOOL "Platform")
endif()
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(AUI_PLATFORM_LINUX 1 CACHE BOOL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS linux)
else()
    set(AUI_PLATFORM_LINUX 0 CACHE BOOL "Platform")
endif()

if (UNIX AND APPLE)
    set(AUI_PLATFORM_APPLE 1 CACHE BOOL "Platform")
else()
    set(AUI_PLATFORM_APPLE 0 CACHE BOOL "Platform")
endif()

if (UNIX AND APPLE AND NOT IOS)
    set(AUI_PLATFORM_MACOS 1 CACHE BOOL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS macos)
else()
    set(AUI_PLATFORM_MACOS 0 CACHE BOOL "Platform")
endif()

if (ANDROID)
    set(AUI_PLATFORM_ANDROID 1 CACHE BOOL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS android)
else()
    set(AUI_PLATFORM_ANDROID 0 CACHE BOOL "Platform")
endif()

if (IOS)
    set(AUI_PLATFORM_IOS 1 CACHE BOOL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS ios)
else()
    set(AUI_PLATFORM_IOS 0 CACHE BOOL "Platform")
endif()

if (UNIX)
    set(AUI_PLATFORM_UNIX 1 CACHE BOOL "Platform")
else()
    set(AUI_PLATFORM_UNIX 0 CACHE BOOL "Platform")
endif()

set(AUI_EXCLUDE_PLATFORMS ${AUI_EXCLUDE_PLATFORMS} CACHE STRING "")

# determine compiler home dir for mingw when crosscompiling
if (MINGW AND CMAKE_CROSSCOMPILING)
    get_filename_component(C_COMPILER_NAME ${CMAKE_C_COMPILER} NAME)
    string(FIND ${C_COMPILER_NAME} "mingw32" _tmp)
    math(EXPR _tmp "${_tmp}+7")
    string(SUBSTRING ${C_COMPILER_NAME} 0 ${_tmp}+7 COMPILER_DIR)
    set(COMPILER_DIR "/usr/${COMPILER_DIR}")
    message(STATUS "Compiler dir is ${COMPILER_DIR}")
endif()


cmake_minimum_required(VERSION 3.16)

if (NOT ANDROID)
    set (CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
    set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
endif()

# mingw winver fix
if (MINGW)
    add_compile_definitions(WINVER=0x601)
endif()

set(AUI_3RDPARTY_LIBS_DIR NOTFOUND CACHE PATH "")
if (AUI_3RDPARTY_LIBS_DIR)
    FILE(GLOB children RELATIVE ${AUI_3RDPARTY_LIBS_DIR} ${AUI_3RDPARTY_LIBS_DIR}/*)
    foreach(child ${children})
        if(IS_DIRECTORY ${AUI_3RDPARTY_LIBS_DIR}/${child})
            list(APPEND CMAKE_PREFIX_PATH ${AUI_3RDPARTY_LIBS_DIR}/${child})
        endif()
    endforeach()
    message("LIBRARY PATH: ${CMAKE_PREFIX_PATH}")
endif()


function(aui_add_properties AUI_MODULE_NAME)
    if(MSVC)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                LINK_FLAGS "/force:MULTIPLE"
                COMPILE_FLAGS "/MP /utf-8")
    endif()

    if(NOT ANDROID)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    endif()
endfunction(aui_add_properties)

macro(_aui_import_gtest)
    if(NOT TARGET GTest::gtest)
        auib_import(GTest https://github.com/google/googletest
                VERSION main
                CMAKE_ARGS -Dgtest_force_shared_crt=TRUE)
        set_property(TARGET GTest::gtest PROPERTY IMPORTED_GLOBAL TRUE)
    endif()
endmacro()

macro(aui_enable_tests)
    _aui_import_gtest()
    if (NOT TARGET GTest::gtest)
        message(FATAL_ERROR "GTest::gtest not found!")
    endif()

    enable_testing()
    if (NOT ANDROID AND NOT IOS)
        get_property(TESTS_SRCS GLOBAL PROPERTY TESTS_SRCS)
        if (NOT TARGET Tests)
            set(TESTS_MODULE_NAME Tests)

            file(WRITE ${CMAKE_BINARY_DIR}/test_main_${TESTS_MODULE_NAME}.cpp [[
#include <gtest/gtest.h>
int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}]])
            add_executable(${TESTS_MODULE_NAME} ${TESTS_SRCS} ${CMAKE_BINARY_DIR}/test_main_${TESTS_MODULE_NAME}.cpp)
            include(GoogleTest)
            gtest_add_tests(TARGET ${TESTS_MODULE_NAME})
            set_property(TARGET ${TESTS_MODULE_NAME} PROPERTY CXX_STANDARD 17)
            target_include_directories(${TESTS_MODULE_NAME} PUBLIC tests)
            get_target_property(_t GTest::gtest INTERFACE_INCLUDE_DIRECTORIES)
            aui_link(${TESTS_MODULE_NAME} PUBLIC GTest::gtest)
            target_compile_definitions(${TESTS_MODULE_NAME} PUBLIC AUI_TESTS_MODULE=1)

            if (TARGET aui.core)
                aui_link(${TESTS_MODULE_NAME} PUBLIC aui.core)
            else()
                aui_link(${TESTS_MODULE_NAME} PUBLIC aui::core)
            endif()

            if (TARGET aui::uitests)
                aui_link(${TESTS_MODULE_NAME} PUBLIC aui::uitests)
            endif()

            aui_add_properties(${TESTS_MODULE_NAME})
            set_target_properties(${TESTS_MODULE_NAME} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
        else()
            target_sources(Tests PRIVATE ${TESTS_SRCS}) # append sources
        endif()
        if (TARGET Tests)
            get_property(TESTS_DEPS GLOBAL PROPERTY TESTS_DEPS)
            get_property(TESTS_INCLUDE_DIRS GLOBAL PROPERTY TESTS_INCLUDE_DIRS)
            foreach(_dep ${TESTS_DEPS})
                get_target_property(_type ${_dep} TYPE)

                # if a target is executable we should add it's dependencies instead of the executable itself since
                # executable's source are already built to our Tests target
                if (_type STREQUAL "EXECUTABLE")
                    get_target_property(_libs ${_dep} LINK_LIBRARIES)
                    get_target_property(_defs ${_dep} COMPILE_DEFINITIONS)
                    aui_link(Tests PRIVATE ${_libs})
                    if (_defs)
                        target_compile_definitions(Tests PRIVATE ${_defs})
                    endif()
                else()
                    aui_link(Tests PRIVATE ${_dep})
                endif()
            endforeach()
            target_include_directories(Tests PRIVATE ${TESTS_INCLUDE_DIRS})
        endif()
        set_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS "")
        set_property(GLOBAL PROPERTY TESTS_SRCS "")
        set_property(GLOBAL PROPERTY TESTS_DEPS "")
    endif()
endmacro()

function(aui_common AUI_MODULE_NAME)
    string(TOLOWER ${AUI_MODULE_NAME} TARGET_NAME)
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)

    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${AUI_MODULE_NAME} PUBLIC AUI_STATIC)
    endif()
    if(AUI_DEBUG)
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_DEBUG)
    else()
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_RELEASE)
        # Strip binary for release builds
        #add_custom_command(TARGET ${AUI_MODULE_NAME} POST_BUILD COMMAND ${CMAKE_STRIP} ${PROJECT_NAME})
    endif()

    if ((UNIX OR MINGW) AND NOT ANDROID AND NOT APPLE)
        target_link_libraries(${AUI_MODULE_NAME} PRIVATE -static-libgcc -static-libstdc++)
    endif()

    # add aui.boot folders to GET_RUNTIME_DEPENDENCIES' paths
    get_property(AUI_BOOT_ROOT_ENTRIES GLOBAL PROPERTY AUI_BOOT_ROOT_ENTRIES)
    unset(DEP_DIRS)
    foreach(_entry ${AUI_BOOT_ROOT_ENTRIES})
        string(REPLACE "=" ";" _entry ${_entry})
        list(GET _entry 1 VAR_VALUE)
        list(APPEND DEP_DIRS ${VAR_VALUE})
    endforeach()

    install(CODE "set(AUI_RUNTIME_DEP_DIRS \"${DEP_DIRS}\")")
    install(CODE "set(AUI_MODULE_NAME \"${AUI_MODULE_NAME}\")")
    install(CODE "set(AUI_MODULE_PATH \"$<TARGET_FILE:${AUI_MODULE_NAME}>\")")
    install(CODE "set(CMAKE_INSTALL_PATH \"${CMAKE_INSTALL_PATH}\")")
    install(CODE "set(CMAKE_PREFIX_PATH \"${CMAKE_PREFIX_PATH}\")")
    install(CODE "set(CMAKE_C_COMPILER \"${CMAKE_C_COMPILER}\")")
    install(CODE "set(COMPILER_DIR \"${COMPILER_DIR}\")")
    install(CODE [[
            message(STATUS "Installing ${AUI_MODULE_NAME}")
    ]])

    # strip
    if (CMAKE_BUILD_TYPE EQUAL "Release")
        install(CODE [[
            set(CMAKE_INSTALL_DO_STRIP TRUE)
        ]])
    endif()

    if (NOT ANDROID AND AUI_INSTALL_RUNTIME_DEPENDENCIES)
        if (MINGW AND CMAKE_CROSSCOMPILING)
            # workaround for crosscompiling on linux/mingw for windows
            # thanks to this thread https://gitlab.kitware.com/cmake/cmake/-/issues/20753
            install(CODE [[
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_PLATFORM "windows+pe")
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_TOOL "objdump")
                set(CMAKE_GET_RUNTIME_DEPENDENCIES_COMMAND "./objdump_unix2dos.sh")
                if (NOT EXISTS "objdump_unix2dos.sh")
                    file(WRITE "objdump_unix2dos.sh" "${CMAKE_OBJDUMP} $@ | unix2dos")
                    cmake_minimum_required(VERSION 3.19)
                    file(CHMOD "objdump_unix2dos.sh" PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_WRITE GROUP_EXECUTE WORLD_READ WORLD_WRITE WORLD_EXECUTE)
                endif()
            ]])
        endif()
        if (BUILD_SHARED_LIBS)
            install(CODE [[
                if (EXISTS ${AUI_MODULE_PATH})
                    set_property(GLOBAL PROPERTY AUI_RESOLVED "")
                    set_property(GLOBAL PROPERTY AUI_UNRESOLVED "")

                    message(STATUS "Installing dependencies for ${AUI_MODULE_NAME}")

                    function(install_dependencies_for MODULE_NAME)
                        if(${MODULE_NAME} STREQUAL "installer")
                            return()
                        endif()

                        set(EXCLUDE_REGEX "^([Cc]:[\\/\\][Ww][Ii][Nn][Dd][Oo][Ww][Ss][\\/\\]).*$")

                        file(GET_RUNTIME_DEPENDENCIES
                             EXECUTABLES
                                 ${MODULE_NAME}
                             LIBRARIES
                                 ${MODULE_NAME}
                             DIRECTORIES
                                ${AUI_RUNTIME_DEP_DIRS}
                             CONFLICTING_DEPENDENCIES_PREFIX CONFLICTING
                             PRE_EXCLUDE_REGEXES ${EXCLUDE_REGEX}
                             POST_EXCLUDE_REGEXES ${EXCLUDE_REGEX}
                             UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED
                             RESOLVED_DEPENDENCIES_VAR RESOLVED
                        )

                        # prefer libraries built by aui.boot over system
                        foreach(_entry ${RESOLVED})
                            if (_entry MATCHES "^\\/(usr|lib).*\\.so.*")
                                get_filename_component(_filename ${_entry} NAME)
                                list(REMOVE_ITEM RESOLVED ${_entry})
                                if (NOT _filename MATCHES "(ld-linux.*|libm|libc)\\.so.*")
                                    list(APPEND UNRESOLVED ${_filename})
                                endif()
                            endif()
                        endforeach()

                        message("install_dependencies_for ${MODULE_NAME} ${UNRESOLVED} ${RESOLVED}")

                        if ("${MODULE_NAME}" STREQUAL "${AUI_MODULE_PATH}")
                            # put additional dependencies
                            foreach (V ${ADDITIONAL_DEPENDENCIES})
                                list(APPEND UNRESOLVED ${V})
                            endforeach()
                        endif()

                        foreach (V ${RESOLVED})
                            message("Resolved[1]: ${V}")
                        endforeach()
                        # try to resolve unresolved dependencies
                        foreach (V ${UNRESOLVED})
                            # avoid duplicates
                            get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
                            list (FIND _tmp ${V} _index)
                            if (${_index} EQUAL -1)
                                if (V MATCHES "^((shell|user|kernel|advapi|ws2_|crypt|wldap|gdi|ole|opengl)32|winmm|dwmapi|msvcrt)\\.dll")
                                    list(REMOVE_ITEM UNRESOLVED ${V})
                                else()
                                    # clear cache entry
                                    unset(TARGET_FILE CACHE)
                                    find_file(
                                        TARGET_FILE
                                            "${V}"
                                        HINTS
                                            "${COMPILER_DIR}"
                                        PATH_SUFFIXES
                                            "bin/"
                                            "lib/"
                                    )
                                    if (EXISTS ${TARGET_FILE})
                                        # add to global resolved items
                                        list(APPEND RESOLVED ${TARGET_FILE})
                                        message("Resolved[2]: ${TARGET_FILE}")
                                        list(REMOVE_ITEM UNRESOLVED ${V})
                                    else()
                                        # add to global unresolved items
                                        # avoid duplicates
                                        get_property(_tmp GLOBAL PROPERTY AUI_UNRESOLVED)
                                        list (FIND _tmp ${V} _index)
                                        if (${_index} EQUAL -1)
                                            list(APPEND _tmp ${V})
                                            set_property(GLOBAL PROPERTY AUI_UNRESOLVED ${_tmp})
                                        endif()
                                    endif()
                                endif()
                            endif()
                        endforeach()
                        get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
                        if (WIN32)
                            set(LIB_DIR "bin")
                        else()
                            set(LIB_DIR "lib")
                        endif()
                        foreach (V ${RESOLVED})
                            list (FIND _tmp ${V} _index)
                            if (${_index} EQUAL -1)
                                list(APPEND _tmp ${V})
                                set_property(GLOBAL PROPERTY AUI_RESOLVED ${_tmp})

                                file(INSTALL
                                     FILES ${V}
                                     TYPE SHARED_LIBRARY
                                     FOLLOW_SYMLINK_CHAIN
                                     DESTINATION "${CMAKE_INSTALL_PREFIX}/${LIB_DIR}"
                                )
                                install_dependencies_for(${V})
                                get_property(_tmp GLOBAL PROPERTY AUI_RESOLVED)
                            endif()
                        endforeach()
                    endfunction()
                    install_dependencies_for(${AUI_MODULE_PATH})
                    get_property(G_RESOLVED GLOBAL PROPERTY AUI_RESOLVED)
                    get_property(G_UNRESOLVED GLOBAL PROPERTY AUI_UNRESOLVED)
                    list(LENGTH G_RESOLVED RESOLVED_LENGTH)
                    if (RESOLVED_LENGTH EQUAL 0)
                        message(WARNING "Count of dependencies of ${AUI_MODULE_NAME} equals to zero which means that "
                                        "something gone wrong in dependency copy script.")

                    endif()

                    list(LENGTH G_UNRESOLVED UNRESOLVED_LENGTH)
                    if (UNRESOLVED_LENGTH GREATER 0)
                        message("There are some unresolved libraries:")
                        foreach (V ${G_UNRESOLVED})
                            message("UNRESOLVED ${V}")
                        endforeach()
                    endif()
                endif()
            ]])
        endif()

        install(CODE [[
            if (EXISTS ${AUI_MODULE_PATH})
                file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/bin" TYPE EXECUTABLE FILES ${AUI_MODULE_PATH})
            endif()
        ]])
    endif()
endfunction(aui_common)


function(aui_deploy_library AUI_MODULE_NAME)
    if (TARGET ${AUI_MODULE_NAME})
        message(STATUS "link_libraries ${AUI_MODULE_NAME}")
        link_libraries(${AUI_MODULE_NAME})
    endif()
    string(TOLOWER ${AUI_MODULE_NAME} AUI_MODULE_NAME_LOWERED)
    install(CODE "list(APPEND ADDITIONAL_DEPENDENCIES ${CMAKE_SHARED_LIBRARY_PREFIX}${AUI_MODULE_NAME_LOWERED}${CMAKE_SHARED_LIBRARY_SUFFIX})")
endfunction(aui_deploy_library)

function(aui_executable AUI_MODULE_NAME)
    file(GLOB_RECURSE SRCS_TESTS_TMP tests/*.cpp tests/*.c tests/*.h)
    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h src/*.mm src/*.m)

    set(options )
    set(oneValueArgs COMPILE_ASSETS EXPORT)
    set(multiValueArgs ADDITIONAL_SRCS)
    cmake_parse_arguments(AUIE "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    # for tests
    # executable's sources
    if (SRCS_TESTS_TMP)
        set_property(GLOBAL APPEND PROPERTY TESTS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/src)
        set_property(GLOBAL APPEND PROPERTY TESTS_SRCS ${SRCS_TESTS_TMP} ${SRCS})
    endif()

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX "(.*\\/)?Platform/${PLATFORM_NAME}\\/.*")
    endforeach()

    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()

        file(GLOB_RECURSE PREVIEW_SRCS preview/*.cpp)
        if (AUI_BUILD_PREVIEW AND PREVIEW_SRCS)
            message(STATUS "Added preview target: ${AUI_MODULE_NAME}")

            set(SRCS ${AUIE_ADDITIONAL_SRCS} ${SRCS})
            set(FILTER_REGEX "(src/main.cpp$|WIN32)")
            set(EXCLUDED_SRCS ${SRCS})
            list(FILTER EXCLUDED_SRCS INCLUDE REGEX ${FILTER_REGEX})
            list(FILTER SRCS EXCLUDE REGEX ${FILTER_REGEX})

            add_executable(${AUI_MODULE_NAME} ${EXCLUDED_SRCS} ${SRCS})
            set_target_properties(${AUI_MODULE_NAME} PROPERTIES ENABLE_EXPORTS ON)
            aui_add_properties(${AUI_MODULE_NAME})
            aui_common(${AUI_MODULE_NAME})

            add_library(preview.${AUI_MODULE_NAME} SHARED ${PREVIEW_SRCS} ${SRCS})
            set_property(TARGET preview.${AUI_MODULE_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)

            target_include_directories(${AUI_MODULE_NAME} PUBLIC src)
            target_link_libraries(preview.${AUI_MODULE_NAME} PUBLIC ${AUI_MODULE_NAME})
            target_link_libraries(preview.${AUI_MODULE_NAME} PUBLIC aui.preview.library)
            aui_add_properties(preview.${AUI_MODULE_NAME})
            aui_common(preview.${AUI_MODULE_NAME})

            add_dependencies(aui.preview preview.${AUI_MODULE_NAME})
        elseif(IOS)
            add_executable(${AUI_MODULE_NAME} MACOSX_BUNDLE ${ADDITIONAL_SRCS} ${SRCS})
        else()
            add_executable(${AUI_MODULE_NAME} ${ADDITIONAL_SRCS} ${SRCS})
        endif()
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    aui_add_properties(${AUI_MODULE_NAME})

    if (SRCS_TESTS_TMP)
        # append executable as a dependency
        set_property(GLOBAL APPEND PROPERTY TESTS_DEPS ${AUI_MODULE_NAME})
    endif()

    aui_common(${AUI_MODULE_NAME})

    if (AUIE_EXPORT)
        install(
                TARGETS ${AUI_MODULE_NAME}
                EXPORT ${AUIE_EXPORT}
                ARCHIVE       DESTINATION "lib"
                LIBRARY       DESTINATION "lib"
                RUNTIME       DESTINATION "bin"
        )

        install(
                DIRECTORY src/
                DESTINATION "${AUI_MODULE_NAME}/include/"
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*.hpp"

        )
    endif()
endfunction(aui_executable)

function(aui_static_link AUI_MODULE_NAME LIBRARY_NAME)
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    target_include_directories(${LIBRARY_NAME} PUBLIC "3rdparty/${LIBRARY_NAME}")
    set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${AUI_MODULE_NAME} PUBLIC ${LIBRARY_NAME})
endfunction(aui_static_link)


macro(_aui_try_find_toolbox)
    find_program(AUI_TOOLBOX_EXE aui.toolbox
            HINTS ${AUI_ROOT}/bin)
    if (NOT AUI_TOOLBOX_EXE)
        file(GLOB_RECURSE AUI_TOOLBOX_EXE ${AUI_CACHE_DIR}/*/aui.toolbox)
        if (AUI_TOOLBOX_EXE)
            list(GET AUI_TOOLBOX_EXE 0 AUI_TOOLBOX_EXE)
        else()
            # compile aui.toolbox for the host system
            message(STATUS "aui.toolbox for the host system is not found - compiling")
        endif()
    endif()
endmacro()

macro(_aui_provide_toolbox_for_host)
    set(_workdir ${CMAKE_CURRENT_BINARY_DIR}/aui_toolbox_provider)
    file(MAKE_DIRECTORY ${_workdir})
    file(MAKE_DIRECTORY ${_workdir}/b)
    file(WRITE ${_workdir}/CMakeLists.txt [[
cmake_minimum_required(VERSION 3.16)

file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

auib_import(AUI https://github.com/aui-framework/aui
            COMPONENTS core toolbox image)
]])
    execute_process(COMMAND ${CMAKE_COMMAND} .. -DAUI_CACHE_DIR=${AUI_CACHE_DIR} WORKING_DIRECTORY ${_workdir}/b RESULT_VARIABLE _r)
    if (NOT _r STREQUAL 0)
        message(FATAL_ERROR "CMake subprocess failed")
    endif()
    _aui_try_find_toolbox()
    set(AUI_TOOLBOX_EXE ${AUI_TOOLBOX_EXE} CACHE FILEPATH "aui.toolbox location")
endmacro()


function(aui_compile_assets AUI_MODULE_NAME)
    cmake_parse_arguments(ASSETS "" "" "EXCLUDE" ${ARGN})
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)

    if(CMAKE_CROSSCOMPILING)
        set(TARGET_DIR ${AUI_SDK_BIN})
    else()
        get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
    endif()


    get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
    file(GLOB_RECURSE ASSETS RELATIVE ${SELF_DIR} "assets/*")

    if (ASSETS_EXCLUDE)
        foreach(_item ${ASSETS})
            string(SUBSTRING ${_item} 7 -1 _path)
            if (_path IN_LIST ASSETS_EXCLUDE)
                list(REMOVE_ITEM ASSETS ${_item})
            endif()
        endforeach()
    endif()

    if (NOT AUI_TOOLBOX_EXE)
        if (CMAKE_CROSSCOMPILING)
            # the worst case because we (possibly) have to compile aui.toolbox for the host system
            _aui_try_find_toolbox()
            if (NOT AUI_TOOLBOX_EXE)
                _aui_provide_toolbox_for_host()
            endif()
        elseif (TARGET aui.toolbox)
            set(AUI_TOOLBOX_EXE $<TARGET_FILE:aui.toolbox> CACHE FILEPATH "aui.toolbox")
        else()
            set(AUI_TOOLBOX_EXE ${AUI_DIR}/bin/aui.toolbox CACHE FILEPATH "aui.toolbox")
        endif()
    endif()
    message(STATUS "aui.toolbox: ${AUI_TOOLBOX_EXE}")
    foreach(ASSET_PATH ${ASSETS})
        string(MD5 OUTPUT_PATH ${ASSET_PATH})
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/autogen/${OUTPUT_PATH}.cpp")
        add_custom_command(
                OUTPUT ${OUTPUT_PATH}
                COMMAND ${AUI_TOOLBOX_EXE} pack ${SELF_DIR}/assets ${SELF_DIR}/${ASSET_PATH} ${OUTPUT_PATH}
                DEPENDS ${SELF_DIR}/${ASSET_PATH}
        )
        target_sources(${AUI_MODULE_NAME} PRIVATE ${OUTPUT_PATH})
    endforeach()
    if(NOT ANDROID)
        if (TARGET aui.toolbox)
            add_dependencies(${AUI_MODULE_NAME} aui.toolbox)
        endif()
    endif()
endfunction(aui_compile_assets)

function(aui_compile_assets_add AUI_MODULE_NAME FILE_PATH ASSET_PATH)
    if(ANDROID)
        set(TARGET_DIR ${AUI_SDK_BIN})
    else()
        get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
    endif()

    if (NOT EXISTS ${FILE_PATH})
        message(FATAL_ERROR "File ${FILE_PATH} does not exist! Is your path absolute?")
    endif()

    if (TARGET aui.toolbox AND NOT CMAKE_CROSSCOMPILING)
        set(AUI_TOOLBOX_EXE $<TARGET_FILE:aui.toolbox>)
    else()
        set(AUI_TOOLBOX_EXE aui.toolbox)
    endif()

    string(MD5 OUTPUT_PATH ${ASSET_PATH})
    set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/autogen/${OUTPUT_PATH}.cpp")
    add_custom_command(
            OUTPUT ${OUTPUT_PATH}
            COMMAND ${AUI_TOOLBOX_EXE} pack_manual ${FILE_PATH} ${ASSET_PATH} ${OUTPUT_PATH}
            DEPENDS ${FILE_PATH}
    )

    target_sources(${AUI_MODULE_NAME} PRIVATE ${OUTPUT_PATH})
    if(NOT ANDROID)
        if (TARGET aui.toolbox)
            add_dependencies(${AUI_MODULE_NAME} aui.toolbox)
        endif()
    endif()
endfunction(aui_compile_assets_add)

function(aui_link AUI_MODULE_NAME) # https://github.com/aui-framework/aui/issues/25
    set(options )
    set(oneValueArgs )
    set(multiValueArgs PRIVATE;PUBLIC;INTERFACE)
    cmake_parse_arguments(AUIL "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    if (NOT BUILD_SHARED_LIBS)
        # static build is a kind of shit where all static libraries' dependencies should be linked to the final exe or
        # dll.
        # thus, some libraries (such as aui.views) require wholearchive linking in order to import all
        # statically-initialized variables (like AStylesheet) to the final execution module.
        foreach(_visibility ${multiValueArgs})
            if (_visibility STREQUAL "PRIVATE")
                set(_public_visibility PUBLIC)
            else()
                set(_public_visibility ${_visibility})
            endif()
            foreach(_dep ${AUIL_${_visibility}})
                # check for wholearchive target flag
                set(_wholearchive OFF)
                if (NOT BUILD_SHARED_LIBS)
                    if (TARGET ${_dep})
                        get_target_property(_wholearchive ${_dep} INTERFACE_AUI_WHOLEARCHIVE)
                    endif()
                endif()
                # set fallback value
                set(_link_target_file ${_dep})
                if (TARGET ${_dep})
                    # adding target's interface include directories and definitions keeping original visibility.
                    get_target_property(_dep_includes ${_dep} INTERFACE_INCLUDE_DIRECTORIES)
                    if (_dep_includes)
                        target_include_directories(${AUI_MODULE_NAME} ${_visibility} ${_dep_includes})
                    endif()

                    get_target_property(_dep_defs ${_dep} INTERFACE_COMPILE_DEFINITIONS)
                    if (_dep_defs)
                        target_compile_definitions(${AUI_MODULE_NAME} ${_visibility} ${_dep_defs})
                    endif()
                endif()
                if (_wholearchive)
                    if (MSVC)
                        # avoid duplicates when using wholearchive
                        get_target_property(_already_linked_libs ${AUI_MODULE_NAME} INTERFACE_LINK_OPTIONS)
                        set(_link_target_file_opt $<TARGET_FILE:${_link_target_file}>)
                        set(_link_target_file_opt "/WHOLEARCHIVE:${_link_target_file_opt}")

                        if (${_link_target_file_opt} IN_LIST _already_linked_libs)
                            continue()
                        endif()

                        # using both target_link_options and target_link_libraries here!!!
                        target_link_options(${AUI_MODULE_NAME} ${_public_visibility} ${_link_target_file_opt})
                    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
                        # avoid duplicates when using wholearchive
                        get_target_property(_already_linked_libs ${AUI_MODULE_NAME} INTERFACE_LINK_LIBRARIES)
                        if (${_link_target_file} IN_LIST _already_linked_libs)
                            continue()
                        endif()
                        if (APPLE)
                            set(_link_target_file -Wl,-force_load ${_link_target_file})
                        else()
                            set(_link_target_file -Wl,--whole-archive,--allow-multiple-definition ${_link_target_file} -Wl,--no-whole-archive)
                        endif()
                    endif()
                endif()

                # linking library preferring public visibility.
                target_link_libraries(${AUI_MODULE_NAME} ${_public_visibility} ${_link_target_file})
            endforeach()
        endforeach()
    else()
        if (AUIL_PRIVATE)
            target_link_libraries(${AUI_MODULE_NAME} PRIVATE ${AUIL_PRIVATE})
        endif()
        if (AUIL_INTERFACE)
            target_link_libraries(${AUI_MODULE_NAME} INTERFACE ${AUIL_INTERFACE})
        endif()
        if (AUIL_PUBLIC)
            target_link_libraries(${AUI_MODULE_NAME} PUBLIC ${AUIL_PUBLIC})
        endif()
    endif()
endfunction()

function(aui_module AUI_MODULE_NAME)
    file(GLOB_RECURSE SRCS_TESTS_TMP tests/*.cpp tests/*.c tests/*.h)


    set(options PLUGIN FORCE_STATIC FORCE_SHARED)
    set(oneValueArgs EXPORT)
    set(multiValueArgs ADDITIONAL_SRCS)
    cmake_parse_arguments(AUIE "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    if (AUIE_FORCE_SHARED AND AUIE_FORCE_STATIC)
        message(FATAL_ERROR "FORCE_SHARED AND FORCE_STATIC flags are exclusive!")
    endif()

    if (SRCS_TESTS_TMP)
        set_property(GLOBAL APPEND PROPERTY TESTS_DEPS ${AUI_MODULE_NAME})
        foreach(child ${SRCS_TESTS_TMP})
            set_property(GLOBAL APPEND PROPERTY TESTS_SRCS ${child})
        endforeach()
    endif()

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.manifest src/*.h src/*.hpp src/*.mm src/*.m)
    if (BUILD_SHARED_LIBS)
        if (WIN32)
            if (EXISTS "${CMAKE_SOURCE_DIR}/Resource.rc")
                set(SRCS ${SRCS} "${CMAKE_SOURCE_DIR}/Resource.rc")
            endif()
        endif()
    endif()

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX ".*\\/${PLATFORM_NAME}\\/.*")
    endforeach()

    if (AUIE_FORCE_SHARED)
        set(AUIE_ADDITIONAL_SRCS SHARED ${AUIE_ADDITIONAL_SRCS})
    endif()
    if (AUIE_FORCE_STATIC)
        set(AUIE_ADDITIONAL_SRCS STATIC ${AUIE_ADDITIONAL_SRCS})
    endif()

    add_library(${AUI_MODULE_NAME} ${AUIE_ADDITIONAL_SRCS} ${SRCS})
    get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
    target_include_directories(${AUI_MODULE_NAME} PUBLIC $<BUILD_INTERFACE:${SELF_DIR}/src>)

    # AUI.Core -> BUILD_AUI_CORE
    string(REPLACE "." "_" BUILD_DEF_NAME ${AUI_MODULE_NAME})
    string(TOUPPER "API_${BUILD_DEF_NAME}" BUILD_DEF_NAME)
    target_compile_definitions(${AUI_MODULE_NAME} INTERFACE ${BUILD_DEF_NAME}=AUI_IMPORT)
    target_compile_definitions(${AUI_MODULE_NAME} PRIVATE ${BUILD_DEF_NAME}=AUI_EXPORT)
    target_compile_definitions(${AUI_MODULE_NAME} PUBLIC GLM_FORCE_INLINE=1)

    aui_add_properties(${AUI_MODULE_NAME})

    string(REPLACE "." "::" BUILD_AS_IMPORTED_NAME ${AUI_MODULE_NAME})

    aui_common(${AUI_MODULE_NAME})
    if (AUIE_EXPORT)
        install(
                TARGETS ${AUI_MODULE_NAME}
                EXPORT ${AUIE_EXPORT}
                ARCHIVE       DESTINATION "${AUI_MODULE_NAME}/lib"
                LIBRARY       DESTINATION "${AUI_MODULE_NAME}/lib"
                RUNTIME       DESTINATION "${AUI_MODULE_NAME}/bin"
                PUBLIC_HEADER DESTINATION "${AUI_MODULE_NAME}/include"
                INCLUDES      DESTINATION "${AUI_MODULE_NAME}/include"
        )

        install(
                DIRECTORY src/
                DESTINATION "${AUI_MODULE_NAME}/include/"
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*.hpp"

        )
    endif()
    if (NOT BUILD_AS_IMPORTED_NAME STREQUAL ${AUI_MODULE_NAME})
        add_library(${BUILD_AS_IMPORTED_NAME} ALIAS ${AUI_MODULE_NAME})
    endif()

    if (AUIE_COMPILE_ASSETS)
        _aui_compile_assets(${AUI_MODULE_NAME})
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)
    endif()

    if (AUIE_PLUGIN)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)

        # define plugin entry for plugins
        if (BUILD_SHARED_LIBS)
            target_compile_definitions(${AUI_MODULE_NAME} PRIVATE _AUI_PLUGIN_ENTRY_N=aui_plugin_entry)
        else()
            target_compile_definitions(${AUI_MODULE_NAME} PRIVATE _AUI_PLUGIN_ENTRY_N=aui_plugin_entry_${BUILD_DEF_NAME})
        endif()
    endif()
endfunction(aui_module)


macro(aui_app)
    # for pulling some resources (cmake scripts, ios storyboards, etc...)
    if (NOT AUI_ROOT)
        foreach(_probe ${CMAKE_CURRENT_SOURCE_DIR} ${AUI_SOURCE_DIR})
            if (EXISTS ${_probe}/aui.build.cmake)
                set(AUI_ROOT ${_probe})
                break()
            endif()
        endforeach()
        if (NOT AUI_ROOT)
            message(FATAL_ERROR "Could not determine aui.build.cmake")
        endif()
    endif()

    set(options NO_INCLUDE_CPACK)
    set(oneValueArgs
            # common
            TARGET
            NAME
            COPYRIGHT
            VERSION
            ICON
            VENDOR

            # linux
            LINUX_DESKTOP

            # apple
            APPLE_TEAM_ID
            APPLE_BUNDLE_IDENTIFIER
            APPLE_SIGN_IDENTITY

            # ios
            IOS_VERSION
            IOS_DEVICE
            IOS_CONTROLLER)
    set(multiValueArgs )
    cmake_parse_arguments(APP "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    # defaults
    # ios
    if (NOT APP_IOS_VERSION)
        set(APP_IOS_VERSION 14.3)
    endif()
    if (NOT APP_IOS_DEVICE)
        set(APP_IOS_DEVICE BOTH)
    endif()
    if (NOT APP_VERSION)
        set(APP_VERSION 1.0)
    endif()
    if (NOT APP_COPYRIGHT)
        set(APP_COPYRIGHT "Copyright is not specified")
    endif()
    if (NOT APP_APPLE_SIGN_IDENTITY)
        set(APP_APPLE_SIGN_IDENTITY "iPhone Developer")
    endif()
    if (NOT APP_IOS_CONTROLLER)
        set(APP_IOS_CONTROLLER "AUIViewController")
    endif()

    unset(_error_msg)
    if (NOT APP_TARGET OR NOT TARGET ${APP_TARGET})
        list(APPEND _error_msg "TARGET which is your app's executable target.")
    endif()
    if (NOT APP_NAME)
        list(APPEND _error_msg "NAME which is your app's display name.")
    endif()
    if (NOT APP_ICON)
        list(APPEND _error_msg_opt "ICON which is path to your app's display icon.")
    endif()
    if (NOT APP_VENDOR)
        list(APPEND _error_msg_opt "VENDOR which is path to your app's developer name.")
    endif()
    list(APPEND _error_msg_opt "COPYRIGHT which is your copyright string (defaults to \"Unknown\").")
    list(APPEND _error_msg_opt "VERSION which is your app's version (defaults to \"1.0\").")
    list(APPEND _error_msg_opt "NO_INCLUDE_CPACK forbids aui_app to include(CPack).")
    if (IOS)
        if (NOT APP_APPLE_TEAM_ID)
            list(APPEND _error_msg "APPLE_TEAM_ID which is your Apple Team ID (https://discussions.apple.com/thread/7942941).")
        endif()
        list(APPEND _error_msg_opt "APPLE_SIGN_IDENTITY which is your code sign identity (defaults to \"iPhone Developer\").")
        list(APPEND _error_msg_opt "IOS_VERSION which is target IOS version (defaults to 14.3).")
        list(APPEND _error_msg_opt "IOS_DEVICE which is target IOS device: either IPHONE, IPAD or BOTH (defaults to BOTH).")
        list(APPEND _error_msg_opt "IOS_CONTROLLER which is your controller name (defaults to AUIViewController)")
    endif()

    if (AUI_PLATFORM_LINUX)
        if (NOT APP_LINUX_DESKTOP_FILE)
            list(APPEND _error_msg_opt "LINUX_DESKTOP_FILE which is your custom *.desktop file")
        endif()
    endif()

    if (_error_msg)
        list(JOIN _error_msg \n v1)
        list(JOIN _error_msg_opt \n v2)
        message(FATAL_ERROR "The following arguments are required for aui_app():\n${v1}\nnote: the following optional variables can be also set:\n${v2}")
    endif()

    # common cpack
    set(_exec \$<TARGET_FILE_NAME:${APP_TARGET}>)
    set(CPACK_PACKAGE_FILE_NAME ${APP_NAME}-${APP_VERSION})
    set(CPACK_BUNDLE_NAME ${APP_NAME})
    set(CPACK_PACKAGE_VENDOR ${APP_VENDOR})
    set(CPACK_BUNDLE_PLIST ${PROJECT_BINARY_DIR}/MacOSXBundleInfo.plist)

    # WINDOWS ==========================================================================================================
    if (AUI_PLATFORM_WINDOWS)
        list(APPEND CPACK_GENERATOR WIX)
    endif()


    # DESKTOP LINUX ====================================================================================================
    if (AUI_PLATFORM_LINUX)
        if (NOT APP_LINUX_DESKTOP)
            # generate desktop file
            set(_exec \$<TARGET_FILE_NAME:${APP_TARGET}>)
            set(_desktop "[Desktop Entry]\nName=${APP_NAME}\nExec=${_exec}\nType=Application\nTerminal=false\nCategories=Utility")
            if (APP_ICON)
                set(_icon "${PROJECT_BINARY_DIR}/app.icon.svg")
                configure_file(${APP_ICON} "${_icon}" COPYONLY)
                set(APP_ICON ${_icon})
                set(_desktop "${_desktop}\nIcon=app.icon")
            endif()
            file(GENERATE
                    OUTPUT "${PROJECT_BINARY_DIR}/app.desktop"
                    CONTENT ${_desktop})
            set(APP_LINUX_DESKTOP ${PROJECT_BINARY_DIR}/app.desktop)
        endif()
        file(GENERATE
                OUTPUT ${PROJECT_BINARY_DIR}/appimage-generate.cmake
                INPUT ${AUI_ROOT}/cmake/appimage-generate.cmake.in)

        file(GENERATE
                OUTPUT ${PROJECT_BINARY_DIR}/appimage-generate-vars.cmake
                CONTENT "set(EXECUTABLE $<TARGET_FILE:${APP_TARGET}>)\nset(DESKTOP_FILE ${APP_LINUX_DESKTOP})\nset(ICON_FILE ${APP_ICON})")
        set(APP_LINUX_DESKTOP ${PROJECT_BINARY_DIR}/appimage-generate.cmake)

        list(APPEND CPACK_GENERATOR External)

        set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${PROJECT_BINARY_DIR}/appimage-generate.cmake")
        set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    endif()

    # IOS AND MACOS ====================================================================================================
    if (APPLE)
        if (NOT APP_APPLE_BUNDLE_IDENTIFIER)
            set(APP_APPLE_BUNDLE_IDENTIFIER ${APP_NAME})
        endif()
        set(PRODUCT_NAME ${APP_NAME})
        set(EXECUTABLE_NAME ${APP_NAME})
        set(MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME})
        set(MACOSX_BUNDLE_INFO_STRING ${APP_APPLE_BUNDLE_IDENTIFIER})
        set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_APPLE_BUNDLE_IDENTIFIER})
        set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_APPLE_BUNDLE_IDENTIFIER})
        set(MACOSX_BUNDLE_ICON_FILE "app.icns")
        set(MACOSX_BUNDLE_LONG_VERSION_STRING ${APP_VERSION})
        set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION})
        set(MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION})
        set(MACOSX_BUNDLE_COPYRIGHT ${APP_COPYRIGHT})
        set(MACOSX_DEPLOYMENT_TARGET ${APP_IOS_VERSION})
        if (AUI_PLATFORM_MACOS)
            configure_file(${AUI_ROOT}/cmake/bundleinfo.plist.in ${CPACK_BUNDLE_PLIST})
        endif()
        set_target_properties(${APP_TARGET} PROPERTIES
                MACOSX_BUNDLE TRUE
                BUNDLE TRUE
                OUTPUT_NAME ${APP_NAME}
                MACOSX_BUNDLE_INFO_PLIST           ${CPACK_BUNDLE_PLIST}
                MACOSX_BUNDLE_EXECUTABLE_NAME      ${MACOSX_BUNDLE_EXECUTABLE_NAME}
                MACOSX_BUNDLE_INFO_STRING          ${MACOSX_BUNDLE_INFO_STRING}
                MACOSX_BUNDLE_GUI_IDENTIFIER       ${MACOSX_BUNDLE_GUI_IDENTIFIER}
                MACOSX_BUNDLE_BUNDLE_NAME          ${MACOSX_BUNDLE_BUNDLE_NAME}
                MACOSX_BUNDLE_ICON_FILE            ${MACOSX_BUNDLE_ICON_FILE}
                MACOSX_BUNDLE_LONG_VERSION_STRING  ${MACOSX_BUNDLE_LONG_VERSION_STRING}
                MACOSX_BUNDLE_SHORT_VERSION_STRING ${MACOSX_BUNDLE_SHORT_VERSION_STRING}
                MACOSX_BUNDLE_BUNDLE_VERSION       ${MACOSX_BUNDLE_BUNDLE_VERSION}
                MACOSX_BUNDLE_COPYRIGHT            ${MACOSX_BUNDLE_COPYRIGHT}
                MACOSX_DEPLOYMENT_TARGET           ${MACOSX_DEPLOYMENT_TARGET}  )
    endif()

    # MACOS ============================================================================================================
    if (AUI_PLATFORM_MACOS)
        configure_file(${AUI_ROOT}/cmake/bundleinfo.plist.in ${CPACK_BUNDLE_PLIST})

        if (APP_ICON)
            # generate icns
            set(_icons_dir ${PROJECT_BINARY_DIR}/app.iconset)
            set(_resolutions 16 32 64 128 256 512 1024)

            unset(_outputs)
            foreach(_res ${_resolutions})
                list(APPEND _outputs "${_icons_dir}/icon_${_res}x${_res}.png")
            endforeach()
            list(JOIN _resolutions , _resolutions_comma)
            if (TARGET aui.toolbox)
                add_dependencies(${APP_TARGET} aui.toolbox)
            endif()
            get_filename_component(_icon_absolute ${APP_ICON} ABSOLUTE)
            set(_icon_icns ${PROJECT_BINARY_DIR}/app.icns)
            add_custom_command(
                    OUTPUT ${_icon_icns}
                    COMMAND ${AUI_TOOLBOX_EXE}
                    ARGS svg2png ${_icon_absolute} -r=${_resolutions_comma} -o=${_icons_dir} -p=icon
                    COMMAND iconutil # iconset to icns
                    ARGS -c icns ${_icons_dir}
            )
            set_source_files_properties(${_icon_icns} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
            target_sources(${APP_TARGET} PRIVATE ${_icon_icns})
        endif()
    endif()

    # IOS ==============================================================================================================
    if (IOS)
        if (APP_IOS_DEVICE STREQUAL IPHONE)
            set(APP_IOS_DEVICE "1")
        elseif (APP_IOS_DEVICE STREQUAL IPAD)
            set(APP_IOS_DEVICE "2")
        elseif (APP_IOS_DEVICE STREQUAL BOTH)
            set(APP_IOS_DEVICE "1,2")
        endif()

        set(LOGIC_ONLY_TESTS 0)                                     # <== Set to 1 if you do not want tests to be hosted by the application, speeds up pure logic tests but you can not run them on real devices

        set(_SYSTEM_PROCESSOR ${CMAKE_SYSTEM_PROCESSOR})
        set(CMAKE_SYSTEM_PROCESSOR ${_SYSTEM_PROCESSOR})
        #include(BundleUtilities)
        include(FindXCTest)
        if (CMAKE_TOOLCHAIN_FILE)
            get_filename_component(CMAKE_TOOLCHAIN_FILE ${CMAKE_TOOLCHAIN_FILE} ABSOLUTE)
        endif()

        message(STATUS XCTestFound:${XCTest_FOUND})

        set(RESOURCES
                ${CMAKE_CURRENT_BINARY_DIR}/Main.storyboard
                ${CMAKE_CURRENT_BINARY_DIR}/LaunchScreen.storyboard
                )

        configure_file(${AUI_ROOT}/cmake/Main.storyboard.in ${CMAKE_CURRENT_BINARY_DIR}/Main.storyboard @ONLY)
        configure_file(${AUI_ROOT}/cmake/LaunchScreen.storyboard.in ${CMAKE_CURRENT_BINARY_DIR}/LaunchScreen.storyboard @ONLY)

        target_sources(${APP_TARGET} PRIVATE ${RESOURCES})
        set_target_properties(${APP_TARGET} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

        # Locate system libraries on iOS
        find_library(UIKIT UIKit REQUIRED)
        find_library(OPENGL OpenGLES REQUIRED)
        find_library(GLKIT GLKit REQUIRED)
        find_library(FOUNDATION Foundation REQUIRED)
        find_library(MOBILECORESERVICES MobileCoreServices REQUIRED)
        find_library(CFNETWORK CFNetwork REQUIRED)
        find_library(SYSTEMCONFIGURATION SystemConfiguration REQUIRED)

        # link the frameworks located above
        target_link_libraries(${APP_TARGET}
                PRIVATE
                ${OPENGL}
                ${UIKIT}
                ${GLKIT}
                ${FOUNDATION}
                ${MOBILECORESERVICES}
                ${CFNETWORK}
                ${SYSTEMCONFIGURATION})


        # Turn on ARC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

        # Create the app target
        set_target_properties(${APP_TARGET} PROPERTIES
                XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
                RESOURCE "${RESOURCES}"
                XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${APP_IOS_VERSION}
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${APP_APPLE_SIGN_IDENTITY}
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${APP_APPLE_TEAM_ID}
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY ${APP_IOS_DEVICE}
                MACOSX_BUNDLE_INFO_PLIST ${AUI_ROOT}/cmake/plist.in
                XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
                XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
                XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
                XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
                XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
                )

        # Include framework headers, needed to make "Build" Xcode action work.
        # "Archive" works fine just relying on default search paths as it has different
        # build product output directory.


        # Set the app's linker search path to the default location on iOS
        set_target_properties(
                ${APP_TARGET}
                PROPERTIES
                XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS
                "@executable_path/Frameworks"
        )

        # Note that commands below are indented just for readability. They will endup as
        # one liners after processing and unescaped ; will disappear so \; are needed.
        # First condition in each command is for normal build, second for archive.
        # \&\>/dev/null makes sure that failure of one command and success of other
        # is not printed and does not make Xcode complain that /bin/sh failed and build
        # continued.

        # Create Frameworks directory in app bundle
        add_custom_command(
                TARGET
                ${APP_TARGET}
                POST_BUILD COMMAND /bin/sh -c
                \"COMMAND_DONE=0 \;
                if ${CMAKE_COMMAND} -E make_directory
                ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_TARGET}.app/Frameworks
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if ${CMAKE_COMMAND} -E make_directory
        \${BUILT_PRODUCTS_DIR}/${APP_TARGET}.app/Frameworks
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if [ \\$$COMMAND_DONE -eq 0 ] \; then
        echo Failed to create Frameworks directory in app bundle \;
        exit 1 \;
        fi\"
        )

        # Copy the framework into the app bundle
        add_custom_command(
                TARGET
                ${APP_TARGET}
                POST_BUILD COMMAND /bin/sh -c
                \"COMMAND_DONE=0 \;
                if ${CMAKE_COMMAND} -E copy_directory
                ${PROJECT_BINARY_DIR}/cppframework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/
                ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_TARGET}.app/Frameworks
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if ${CMAKE_COMMAND} -E copy_directory
        \${BUILT_PRODUCTS_DIR}/${FRAMEWORK_NAME}.framework
        \${BUILT_PRODUCTS_DIR}/${APP_TARGET}.app/Frameworks/${FRAMEWORK_NAME}.framework
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if [ \\$$COMMAND_DONE -eq 0 ] \; then
        echo Failed to copy the framework into the app bundle \;
        exit 1 \;
        fi\"
        )

        # Codesign the framework in it's new spot
        add_custom_command(
                TARGET
                ${APP_TARGET}
                POST_BUILD COMMAND /bin/sh -c
                \"COMMAND_DONE=0 \;
                if codesign --force --verbose
                ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_TARGET}.app/Frameworks/${FRAMEWORK_NAME}.framework
                --sign ${APP_APPLE_SIGN_IDENTITY}
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if codesign --force --verbose
        \${BUILT_PRODUCTS_DIR}/${APP_TARGET}.app/Frameworks/${FRAMEWORK_NAME}.framework
        --sign ${APP_APPLE_SIGN_IDENTITY}
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if [ \\$$COMMAND_DONE -eq 0 ] \; then
        echo Framework codesign failed \;
        exit 1 \;
        fi\"
        )

        # Add a "PlugIns" folder as a kludge fix for how the XcTest package generates paths
        add_custom_command(
                TARGET
                ${APP_TARGET}
                POST_BUILD COMMAND /bin/sh -c
                \"COMMAND_DONE=0 \;
                if ${CMAKE_COMMAND} -E make_directory
                ${PROJECT_BINARY_DIR}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/PlugIns
        \&\>/dev/null \; then
        COMMAND_DONE=1 \;
        fi \;
        if [ \\$$COMMAND_DONE -eq 0 ] \; then
        echo Failed to create PlugIns directory in EFFECTIVE_PLATFORM_NAME folder. \;
        exit 1 \;
        fi\"
        )
    endif()
    if (NOT APP_NO_INCLUDE_CPACK)
        include(CPack)
    endif()
endmacro()

if (MINGW OR UNIX)
    # strip for release
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
endif()


# Coverage support
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    if (UNIX)
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#    endif()
#endif()
