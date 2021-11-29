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

# generator expressions for install(CODE [[ ... ]])
set(CMAKE_POLICY_DEFAULT_CMP0087 NEW)
set(AUI_BUILD_PREVIEW OFF CACHE BOOL "Enable aui.preview plugin target")
cmake_policy(SET CMP0072 NEW)

# platform definitions
# platform exclusion (AUI/Platform/<platform name>/...)
set(AUI_EXCLUDE_PLATFORMS android linux macos win32)

if (WIN32)
    set(AUI_PLATFORM_WIN 1)
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS win32)
else()
    set(AUI_PLATFORM_WIN 0)
endif()
if (UNIX AND NOT APPLE AND NOT ANDROID)
    set(AUI_PLATFORM_LINUX 1)
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS linux)
else()
    set(AUI_PLATFORM_LINUX 0)
endif()

if (UNIX AND APPLE)
    set(AUI_PLATFORM_APPLE 1)
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS macos)
else()
    set(AUI_PLATFORM_APPLE 0)
endif()

if (ANDROID)
    set(AUI_PLATFORM_ANDROID 1)
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS android)
else()
    set(AUI_PLATFORM_ANDROID 0)
endif()

if (UNIX)
    set(AUI_PLATFORM_UNIX 1)
else()
    set(AUI_PLATFORM_UNIX 0)
endif()


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

function(aui_tests TESTS_MODULE_NAME)
    find_package(Boost)
    if(Boost_FOUND)
        add_executable(${ARGV})
        set_property(TARGET ${TESTS_MODULE_NAME} PROPERTY CXX_STANDARD 17)
        target_include_directories(${TESTS_MODULE_NAME} PUBLIC tests)
        add_definitions(-DBOOST_ALL_NO_LIB)
        target_include_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_INCLUDE_DIRS})
        target_link_directories(${TESTS_MODULE_NAME} PRIVATE ${Boost_LIBRARY_DIRS})

        if (TARGET aui.core)
            target_link_libraries(${TESTS_MODULE_NAME} PRIVATE aui.core)
        else()
            target_link_libraries(${TESTS_MODULE_NAME} PRIVATE aui::core)
        endif()

        target_link_libraries(${TESTS_MODULE_NAME} PRIVATE ${AUI_MODULE_NAME})
        aui_add_properties(${TESTS_MODULE_NAME})
        set_target_properties(${TESTS_MODULE_NAME} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
    else()
        message(WARNING "Boost was not found! Test target is not available.")
    endif()
endfunction(aui_tests)

function(aui_common AUI_MODULE_NAME)
    string(TOLOWER ${AUI_MODULE_NAME} TARGET_NAME)
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 17)
    file(GLOB_RECURSE SRCS_TESTS_TMP tests/*.cpp tests/*.c tests/*.h)

    if (SRCS_TESTS_TMP)
        set_property(GLOBAL APPEND PROPERTY TESTS_DEPS ${AUI_MODULE_NAME})
        foreach(child ${SRCS_TESTS_TMP})
            set_property(GLOBAL APPEND PROPERTY TESTS_SRCS ${child})
        endforeach()
    endif()
    if(AUI_STATIC)
        target_compile_definitions(${AUI_MODULE_NAME} INTERFACE AUI_STATIC)
    endif()
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
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

    if (NOT ANDROID)
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

function(aui_executable_advanced AUI_MODULE_NAME ADDITIONAL_SRCS)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.h)

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX ".*\\/${PLATFORM_NAME}\\/.*")
    endforeach()

    #message("ASSDIR ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp")
    if(ANDROID)
        add_library(${AUI_MODULE_NAME} SHARED ${SRCS})
    else()

        file(GLOB_RECURSE PREVIEW_SRCS preview/*.cpp)
        if (AUI_BUILD_PREVIEW AND PREVIEW_SRCS)
            message(STATUS "Added preview target: ${AUI_MODULE_NAME}")

            set(SRCS ${ADDITIONAL_SRCS} ${SRCS})
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
        else()
            add_executable(${AUI_MODULE_NAME} ${ADDITIONAL_SRCS} ${SRCS})
        endif()
    endif()

    target_include_directories(${AUI_MODULE_NAME} PRIVATE src)

    aui_add_properties(${AUI_MODULE_NAME})

    aui_common(${AUI_MODULE_NAME})
endfunction(aui_executable_advanced)

function(aui_executable AUI_MODULE_NAME)
    aui_executable_advanced(${AUI_MODULE_NAME} "")
endfunction(aui_executable)

function(aui_static_link AUI_MODULE_NAME LIBRARY_NAME)
    target_include_directories(${AUI_MODULE_NAME} PUBLIC "3rdparty/${LIBRARY_NAME}")
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${AUI_MODULE_NAME} PUBLIC ${LIBRARY_NAME})
endfunction(aui_static_link)


function(aui_compile_assets AUI_MODULE_NAME)
    cmake_parse_arguments(ASSETS "" "" "EXCLUDE" ${ARGN})

    if(ANDROID)
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
            # FIXME assume that aui.toolbox is already built for our system
            find_program(AUI_TOOLBOX_EXE aui.toolbox
                         HINTS ${AUI_DIR}/bin
                         REQUIRED)
        elseif (TARGET aui.toolbox)
            set(AUI_TOOLBOX_EXE $<TARGET_FILE:aui.toolbox> CACHE FILEPATH "aui.toolbox")
        else()
            set(AUI_TOOLBOX_EXE ${AUI_DIR}/bin/aui.toolbox CACHE FILEPATH "aui.toolbox")
        endif()
        message(STATUS "aui.toolbox: ${AUI_TOOLBOX_EXE}")
    endif()
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

function(aui_module AUI_MODULE_NAME)
    project(${AUI_MODULE_NAME})

    file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_BINARY_DIR}/autogen/*.cpp src/*.cpp src/*.c src/*.manifest src/*.h src/*.hpp)
    if (WIN32)
        if (EXISTS "${CMAKE_SOURCE_DIR}/Resource.rc")
            set(SRCS ${SRCS} "${CMAKE_SOURCE_DIR}/Resource.rc")
        endif()
    endif()

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX ".*\\/${PLATFORM_NAME}\\/.*")
    endforeach()

    add_library(${AUI_MODULE_NAME} SHARED ${SRCS} ${ARGN})
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
    add_library(${BUILD_AS_IMPORTED_NAME} ALIAS ${AUI_MODULE_NAME})

    aui_common(${AUI_MODULE_NAME})
    install(
            TARGETS ${AUI_MODULE_NAME}
            EXPORT AUI
            ARCHIVE
            DESTINATION "${AUI_MODULE_NAME}/lib"
            LIBRARY
            DESTINATION "${AUI_MODULE_NAME}/lib"
            RUNTIME
            DESTINATION "${AUI_MODULE_NAME}/bin"
            PUBLIC_HEADER DESTINATION "${AUI_MODULE_NAME}/include"
    )

    install(
            DIRECTORY src/
            DESTINATION "${AUI_MODULE_NAME}/include/"
            FILES_MATCHING PATTERN "*.h"
            PATTERN "*.hpp"

    )
endfunction(aui_module)

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
