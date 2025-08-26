# SPDX-License-Identifier: MPL-2.0
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# CMake AUI building functions

include_guard(GLOBAL)

define_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS
        BRIEF_DOCS "Global list of test include dirs"
        FULL_DOCS "Global list of test include dirs")
define_property(GLOBAL PROPERTY TESTS_DEPS
        BRIEF_DOCS "Global list of test dependencies"
        FULL_DOCS "Global list of test dependencies")
define_property(GLOBAL PROPERTY TESTS_EXECUTABLES
        BRIEF_DOCS "Global list of test executables"
        FULL_DOCS "Global list of test executables")

define_property(TARGET PROPERTY AUI_INSTALL_RUNTIME_DIR
        BRIEF_DOCS "Directory for runtimes installation (exe, dll, linux executables), defaults to \"bin\""
        FULL_DOCS "Directory for runtimes installation (exe, dll, linux executables), defaults to \"bin\""
)

define_property(TARGET PROPERTY INTERFACE_AUI_WHOLEARCHIVE
        BRIEF_DOCS "Use wholearchive when linking this library to another"
        FULL_DOCS "Use wholearchive when linking this library to another")

set_property(GLOBAL PROPERTY TESTS_INCLUDE_DIRS "")

if (AUIB_TRACE_BUILD_SYSTEM)
    message(STATUS "AUIB_TRACE_BUILD_SYSTEM=TRUE (build system verbose logging is enabled)")
endif()

message(STATUS "CMake Version: ${CMAKE_VERSION}")

# generator expressions for install(CODE [[ ... ]])
set(CMAKE_POLICY_DEFAULT_CMP0087 NEW)
set(AUI_BUILD_PREVIEW OFF CACHE BOOL "Enable aui.preview plugin target")
set(AUI_BUILD_FOR "" CACHE STRING "Specifies target cross-compilation platform")
set(AUI_INSTALL_RUNTIME_DEPENDENCIES ON CACHE BOOL "Install runtime dependencies along with the project")
set(CMAKE_CXX_STANDARD 20)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/helpers")

cmake_policy(SET CMP0072 NEW)

if (CMAKE_CROSSCOMPILING AND AUI_BUILD_FOR)
    message(FATAL_ERROR "CMAKE_CROSSCOMPILING and AUI_BUILD_FOR are exclusive vars.")
endif()

if (ANDROID OR IOS)
    set(_build_shared OFF)
    message(STATUS "Forcing static build because you are building for mobile platform.")
else()
    set(_build_shared ON)
endif()
set(BUILD_SHARED_LIBS ${_build_shared} CACHE BOOL "Build using shared libraries")

# platform definitions
# platform exclusion (AUI/Platform/<platform name>/...)
set(AUI_EXCLUDE_PLATFORMS android linux macos win32 ios apple unix emscripten)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(AUI_DEBUG TRUE)
else()
    set(AUI_DEBUG FALSE)
endif()

# platforms
if (WIN32)
    set(AUI_PLATFORM_WIN 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS win32)
else()
    set(AUI_PLATFORM_WIN 0 CACHE INTERNAL "Platform")
endif()
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(AUI_PLATFORM_LINUX 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS linux)
else()
    set(AUI_PLATFORM_LINUX 0 CACHE INTERNAL "Platform")
endif()

if (UNIX AND APPLE)
    set(AUI_PLATFORM_APPLE 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS apple)
else()
    set(AUI_PLATFORM_APPLE 0 CACHE INTERNAL "Platform")
endif()

if (UNIX AND APPLE AND NOT IOS)
    set(AUI_PLATFORM_MACOS 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS macos)
else()
    set(AUI_PLATFORM_MACOS 0 CACHE INTERNAL "Platform")
endif()

if (ANDROID)
    set(AUI_PLATFORM_ANDROID 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS android)
else()
    set(AUI_PLATFORM_ANDROID 0 CACHE INTERNAL "Platform")
endif()

if (IOS)
    set(AUI_PLATFORM_IOS 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS ios)
else()
    set(AUI_PLATFORM_IOS 0 CACHE INTERNAL "Platform")
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    set(AUI_PLATFORM_EMSCRIPTEN 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS emscripten)
else()
    set(AUI_PLATFORM_EMSCRIPTEN 0 CACHE INTERNAL "Platform")
endif()

if (UNIX AND NOT AUI_PLATFORM_EMSCRIPTEN)
    set(AUI_PLATFORM_UNIX 1 CACHE INTERNAL "Platform")
    list(REMOVE_ITEM AUI_EXCLUDE_PLATFORMS unix)
else()
    set(AUI_PLATFORM_UNIX 0 CACHE INTERNAL "Platform")
endif()

# compilers
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(AUI_COMPILER_CLANG 1 CACHE INTERNAL "Compiler")
else()
    set(AUI_COMPILER_CLANG 0 CACHE INTERNAL "Compiler")
endif()

if (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    set(AUI_COMPILER_GCC 1 CACHE INTERNAL "Compiler")
else()
    set(AUI_COMPILER_GCC 0 CACHE INTERNAL "Compiler")
endif()

if (MSVC)
    set(AUI_COMPILER_MSVC 1 CACHE INTERNAL "Compiler")
else()
    set(AUI_COMPILER_MSVC 0 CACHE INTERNAL "Compiler")
endif()

if (CMAKE_GENERATOR_PLATFORM MATCHES "(arm64)|(ARM64)" OR CMAKE_SYSTEM_PROCESSOR MATCHES "(aarch64|arm64)")
    set(AUI_ARCH_X86_64 0 CACHE INTERNAL "Arch")
    set(AUI_ARCH_X86 0 CACHE INTERNAL "Arch")
    set(AUI_ARCH_ARM_64 1 CACHE INTERNAL "Arch")
    set(AUI_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "armv7-a")
    set(AUI_ARCH_X86_64 0 CACHE INTERNAL "Arch")
    set(AUI_ARCH_X86 0 CACHE INTERNAL "Arch")
    set(AUI_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
    set(AUI_ARCH_ARM_V7 1 CACHE INTERNAL "Arch")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)|(i.86)")
    if (CMAKE_SIZEOF_VOID_P STREQUAL 8)
        set(AUI_ARCH_X86_64 1 CACHE INTERNAL "Arch")
        set(AUI_ARCH_X86 0 CACHE INTERNAL "Arch")
        set(AUI_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
        set(AUI_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
    else()
        set(AUI_ARCH_X86_64 0 CACHE INTERNAL "Arch")
        set(AUI_ARCH_X86 1 CACHE INTERNAL "Arch")
        set(AUI_ARCH_ARM_64 0 CACHE INTERNAL "Arch")
        set(AUI_ARCH_ARM_V7 0 CACHE INTERNAL "Arch")
    endif()
endif()

set(AUI_EXCLUDE_PLATFORMS ${AUI_EXCLUDE_PLATFORMS} CACHE INTERNAL "")

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
        if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                    LINK_FLAGS "/force:MULTIPLE"
                    COMPILE_FLAGS "/MP /utf-8")
        else() # clang-cl does not support /MP
            set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                    LINK_FLAGS "/force:MULTIPLE"
                    COMPILE_FLAGS "/utf-8")
        endif()
    endif()

    if(NOT ANDROID)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES
                ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
                RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
    endif()
endfunction(aui_add_properties)

# gtest
macro(_aui_import_gtest)
    if (NOT TARGET GTest::gtest)
        if(AUIB_DISABLE)
            find_package(GTest REQUIRED CONFIG)
        else()
            if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
                auib_import(GTest https://github.com/google/googletest
                    VERSION v1.17.0
                    CMAKE_ARGS -Dgtest_force_shared_crt=FALSE
                    LINK STATIC) # Enforce /MT
            else()
                auib_import(GTest https://github.com/google/googletest
                            VERSION v1.17.0
                            CMAKE_ARGS -Dgtest_force_shared_crt=TRUE
                            LINK STATIC)
            endif()
        endif()
        set_property(TARGET GTest::gtest PROPERTY IMPORTED_GLOBAL TRUE)
        set_property(TARGET GTest::gmock PROPERTY IMPORTED_GLOBAL TRUE)
    endif()
endmacro()

macro(aui_enable_tests AUI_MODULE_NAME)
    if (NOT CMAKE_CROSSCOMPILING)
        _aui_import_gtest()
        if (NOT TARGET GTest::gtest)
            message(FATAL_ERROR "GTest::gtest not found!")
        endif()

        enable_testing()
        get_property(_source_dir TARGET ${AUI_MODULE_NAME} PROPERTY SOURCE_DIR)
        unset(TESTS_SRCS)

        if (NOT EXISTS ${_source_dir}/tests)
            message(FATAL_ERROR "aui_enable_tests expects ${_source_dir}/tests to exist")
        endif()
        file(GLOB_RECURSE TESTS_SRCS ${_source_dir}/tests/*.cpp)

        if (NOT TARGET Tests)
            set(TESTS_MODULE_NAME Tests)

            file(WRITE ${CMAKE_BINARY_DIR}/test_main_${TESTS_MODULE_NAME}.cpp [[
    #include <gmock/gmock.h>
    int main(int argc, char **argv) {
#ifdef __linux
#ifdef AUI_CATCH_UNHANDLED
    extern void aui_init_signal_handler();
    aui_init_signal_handler();
#endif
#endif
    // Since Google Mock depends on Google Test, InitGoogleMock() is
    // also responsible for initializing Google Test.  Therefore there's
    testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
    }]])
            add_executable(${TESTS_MODULE_NAME} ${TESTS_SRCS} ${CMAKE_BINARY_DIR}/test_main_${TESTS_MODULE_NAME}.cpp)
#            _auib_apply_rpath(${TESTS_MODULE_NAME})
            include(GoogleTest)
            #gtest_add_tests(TARGET ${TESTS_MODULE_NAME})
            set_property(TARGET ${TESTS_MODULE_NAME} PROPERTY CXX_STANDARD 20)
            target_include_directories(${TESTS_MODULE_NAME} PUBLIC tests)
            target_link_libraries(${TESTS_MODULE_NAME} PUBLIC GTest::gmock)

            target_compile_definitions(${TESTS_MODULE_NAME} PUBLIC AUI_TESTS_MODULE=1)

            if (TARGET aui::core)
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
            # if the specified target is an executable, we should add it's srcs to the Tests target, otherwise just link it.
            get_property(_type TARGET ${AUI_MODULE_NAME} PROPERTY TYPE)
            if (_type STREQUAL EXECUTABLE)
                get_target_property(_t ${AUI_MODULE_NAME} SOURCES)

                # remove unexisting sources; otherwise cmake would fail for unknown reason
                foreach(_e ${_t})
                    if (NOT EXISTS ${_e})
                        list(REMOVE_ITEM _t ${_e})
                    endif()
                endforeach()

                target_sources(Tests PRIVATE ${_t})
                get_target_property(_t ${AUI_MODULE_NAME} INCLUDE_DIRECTORIES)
                target_include_directories(Tests PRIVATE ${_t})
                get_target_property(_t ${AUI_MODULE_NAME} COMPILE_DEFINITIONS)
                if(_t)
                    target_compile_definitions(Tests PRIVATE ${_t})
                endif()
                get_target_property(_t ${AUI_MODULE_NAME} LINK_LIBRARIES)
                aui_link(Tests PRIVATE ${_t})
            else()
                aui_link(Tests PRIVATE ${AUI_MODULE_NAME})
            endif()
        endif()
    endif()
endmacro()

# google benchmark

macro(_aui_import_google_benchmark)
    if (NOT TARGET benchmark::benchmark)
        if(NOT(MSVC AND AUI_BUILD_FOR STREQUAL "winxp"))
            if(AUIB_DISABLE)
                find_package(benchmark REQUIRED CONFIG)
            else()
                auib_import(benchmark https://github.com/google/benchmark
                            VERSION v1.8.3
                            CMAKE_ARGS -DBENCHMARK_ENABLE_GTEST_TESTS=OFF
                            LINK STATIC)
            endif()
            set_property(TARGET benchmark::benchmark PROPERTY IMPORTED_GLOBAL TRUE)
        endif()
    endif()
endmacro()

macro(aui_enable_benchmarks AUI_MODULE_NAME)
    if (NOT CMAKE_CROSSCOMPILING)
        _aui_import_gtest()
        _aui_import_google_benchmark()
        if (NOT TARGET benchmark::benchmark)
            message(FATAL_ERROR "benchmark::benchmark not found!")
        endif()

        get_property(_source_dir TARGET ${AUI_MODULE_NAME} PROPERTY SOURCE_DIR)
        unset(benchmarks_SRCS)

        if (NOT EXISTS ${_source_dir}/benchmarks)
            message(FATAL_ERROR "aui_enable_benchmarks expects ${_source_dir}/benchmarks to exist")
        endif()
        file(GLOB_RECURSE benchmarks_SRCS ${_source_dir}/benchmarks/*.cpp)

        if (NOT TARGET Benchmarks)
            set(benchmarks_MODULE_NAME Benchmarks)

            file(WRITE ${CMAKE_BINARY_DIR}/benchmarks_main_${benchmarks_MODULE_NAME}.cpp [[
    #include <benchmark/benchmark.h>
    int main(int argc, char** argv)
    {
        ::benchmark::Initialize(&argc, argv);
        ::benchmark::RunSpecifiedBenchmarks();
    }
    ]])
            add_executable(${benchmarks_MODULE_NAME} ${benchmarks_SRCS} ${CMAKE_BINARY_DIR}/benchmarks_main_${benchmarks_MODULE_NAME}.cpp)
            _auib_apply_rpath(${benchmarks_MODULE_NAME})
            set_property(TARGET ${benchmarks_MODULE_NAME} PROPERTY CXX_STANDARD 20)
            target_include_directories(${benchmarks_MODULE_NAME} PUBLIC benchmarks)
            target_link_libraries(${benchmarks_MODULE_NAME} PUBLIC benchmark::benchmark benchmark::benchmark_main)

            target_compile_definitions(${benchmarks_MODULE_NAME} PUBLIC AUI_benchmarks_MODULE=1)

            if (TARGET aui::core)
                aui_link(${benchmarks_MODULE_NAME} PUBLIC aui::core)
            endif()

            if (TARGET aui::uibenchmarks)
                aui_link(${benchmarks_MODULE_NAME} PUBLIC aui::uibenchmarks)
            endif()

            aui_add_properties(${benchmarks_MODULE_NAME})
            set_target_properties(${benchmarks_MODULE_NAME} PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)
        else()
            target_sources(Benchmarks PRIVATE ${benchmarks_SRCS}) # append sources
        endif()

        if (TARGET Benchmarks)
            # if the specified target is an executable, we should add it's srcs to the Benchmarks target, otherwise just link it.
            get_property(_type TARGET ${AUI_MODULE_NAME} PROPERTY TYPE)
            if (_type STREQUAL EXECUTABLE)
                get_target_property(_t ${AUI_MODULE_NAME} SOURCES)

                # remove unexisting sources; otherwise cmake would fail for unknown reason
                foreach(_e ${_t})
                    if (NOT EXISTS ${_e})
                        list(REMOVE_ITEM _t ${_e})
                    endif()
                endforeach()

                target_sources(Benchmarks PRIVATE ${_t})
                get_target_property(_t ${AUI_MODULE_NAME} INCLUDE_DIRECTORIES)
                target_include_directories(Benchmarks PRIVATE ${_t})
                get_target_property(_t ${AUI_MODULE_NAME} COMPILE_DEFINITIONS)
                if(_t)
                    target_compile_definitions(Benchmarks PRIVATE ${_t})
                endif()
                get_target_property(_t ${AUI_MODULE_NAME} LINK_LIBRARIES)
                aui_link(Benchmarks PRIVATE ${_t})
            else()
                aui_link(Benchmarks PRIVATE ${AUI_MODULE_NAME})
            endif()
        endif()
    endif()
endmacro()

# [_auib_apply_rpath]
function(_auib_apply_rpath AUI_MODULE_NAME)
    if (BUILD_SHARED_LIBS AND AUI_PLATFORM_LINUX)
        set(_rpath "$ORIGIN;$ORIGIN/../lib")
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES BUILD_RPATH "${_rpath}"
                INSTALL_RPATH "${_rpath}")
    endif()
endfunction()
# [_auib_apply_rpath]

# common function fo aui_executable and aui_module
function(aui_common AUI_MODULE_NAME)
    string(TOLOWER ${AUI_MODULE_NAME} TARGET_NAME)
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES OUTPUT_NAME ${TARGET_NAME})
    set_property(TARGET ${AUI_MODULE_NAME} PROPERTY CXX_STANDARD 20)

    target_compile_definitions(${AUI_MODULE_NAME} PRIVATE AUI_MODULE_NAME=${AUI_MODULE_NAME})
    target_compile_definitions(${AUI_MODULE_NAME} PRIVATE AUI_CMAKE_PROJECT_VERSION=${CMAKE_PROJECT_VERSION})

    _auib_apply_rpath(${AUI_MODULE_NAME})

    if(NOT BUILD_SHARED_LIBS)
        target_compile_definitions(${AUI_MODULE_NAME} PUBLIC AUI_STATIC)
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

    set_target_properties(${AUI_MODULE_NAME} PROPERTIES AUI_INSTALL_RUNTIME_DIR "bin")

    install(TARGETS ${APP_TARGET}
            DESTINATION ".")
    install(CODE "set(AUI_INSTALL_RUNTIME_DIR \"\${CMAKE_INSTALL_PREFIX}/$<TARGET_PROPERTY:${AUI_MODULE_NAME},AUI_INSTALL_RUNTIME_DIR>\")")

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

                        set(_install_runtime_dir ${AUI_INSTALL_RUNTIME_DIR})
                        if (NOT WIN32)
                            set(_install_runtime_dir "${_install_runtime_dir}/../lib")
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
                                     DESTINATION ${_install_runtime_dir}
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

function(_auib_collect_srcs _out dir)
    file(GLOB_RECURSE SRCS
            ${dir}/*.cpp
            ${dir}/*.c
            ${dir}/*.mm
            ${dir}/*.m)

    set(${_out} ${SRCS} PARENT_SCOPE)
endfunction()

function(auib_add_srcs TARGET DIR)
    _auib_collect_srcs(_src ${DIR})
    target_sources(${TARGET} PRIVATE ${_src})
    target_include_directories(${TARGET} PRIVATE ${DIR})
endfunction()

function(aui_executable AUI_MODULE_NAME)
    _auib_collect_srcs(SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/)
    _auib_collect_srcs(SRCS_TESTS_TMP ${CMAKE_CURRENT_SOURCE_DIR}/tests/)

    set(options WIN32_SUBSYSTEM_CONSOLE)
    set(oneValueArgs COMPILE_ASSETS EXPORT)
    set(multiValueArgs ADDITIONAL_SRCS)
    cmake_parse_arguments(AUIE "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX "(.*\\/)?[Pp]latform/${PLATFORM_NAME}\\/.*")
    endforeach()

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
        elseif(APPLE)
            add_executable(${AUI_MODULE_NAME} MACOSX_BUNDLE ${ADDITIONAL_SRCS} ${SRCS})
        elseif(WIN32 AND NOT AUIE_WIN32_SUBSYSTEM_CONSOLE)
            add_executable(${AUI_MODULE_NAME} WIN32 ${ADDITIONAL_SRCS} ${SRCS})
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

    if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
        include(WinXPuseLTL5)
        include(WinXPuseYYThunks)
        target_link_libraries(${AUI_MODULE_NAME} PRIVATE YY_Thunks)
        if (AUIE_WIN32_SUBSYSTEM_CONSOLE)
            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01")
            else()
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.02")
            endif()
        else()
            if (CMAKE_SIZEOF_VOID_P EQUAL 4)
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01")
            else()
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.02")
            endif()
        endif()
    endif()

    if (AUIE_EXPORT)
        install(
                TARGETS ${AUI_MODULE_NAME}
                EXPORT ${AUIE_EXPORT}
                ARCHIVE       DESTINATION "lib"
                LIBRARY       DESTINATION "lib"
                RUNTIME       DESTINATION "$<TARGET_PROPERTY:${AUI_MODULE_NAME},AUI_INSTALL_RUNTIME_DIR>"
                BUNDLE        DESTINATION "bin"
        )

        install(
                DIRECTORY src/
                DESTINATION "${AUI_MODULE_NAME}/include/"
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*.hpp"

        )
    endif()

    if (AUI_PLATFORM_EMSCRIPTEN)
        configure_file(${AUI_BUILD_AUI_ROOT}/platform/emscripten/index.html.in ${CMAKE_BINARY_DIR}/bin/${AUI_MODULE_NAME}.html)
    endif()
endfunction(aui_executable)

function(aui_static_link AUI_MODULE_NAME LIBRARY_NAME)
    file(GLOB_RECURSE SRCS "3rdparty/${LIBRARY_NAME}/*.cpp" "3rdparty/${LIBRARY_NAME}/*.c" "3rdparty/${LIBRARY_NAME}/*.h")
    add_library(${LIBRARY_NAME} STATIC ${SRCS})
    target_include_directories(${LIBRARY_NAME} PUBLIC "3rdparty/${LIBRARY_NAME}")
    set_property(TARGET ${LIBRARY_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
    target_link_libraries(${AUI_MODULE_NAME} PUBLIC ${LIBRARY_NAME})
endfunction(aui_static_link)

macro(_aui_find_root)
    if (NOT AUI_BUILD_AUI_ROOT)
        set(AUI_BUILD_AUI_ROOT ${AUI_ROOT})
        if (NOT AUI_BUILD_AUI_ROOT)
            message(FATAL_ERROR "Could not find AUI root")
        endif()
    endif()
endmacro()

macro(_aui_try_find_toolbox)
    _aui_find_root()
    message(STATUS "AUI_BUILD_AUI_ROOT = ${AUI_BUILD_AUI_ROOT}")
    find_program(AUI_TOOLBOX_EXE aui.toolbox
            HINTS ${AUI_BUILD_AUI_ROOT}/bin)
    if (NOT AUI_TOOLBOX_EXE)
        file(GLOB_RECURSE AUI_TOOLBOX_EXE ${AUIB_CACHE_DIR}/crosscompile-host/prefix/aui.toolbox.exe ${AUIB_CACHE_DIR}/crosscompile-host/prefix/aui.toolbox)

        if (AUI_TOOLBOX_EXE)
            list(GET AUI_TOOLBOX_EXE 0 AUI_TOOLBOX_EXE)
            set(AUI_TOOLBOX_EXE ${AUI_TOOLBOX_EXE} CACHE FILEPATH "aui.toolbox location" FORCE)
        endif()
    endif()
endmacro()

macro(_aui_provide_toolbox_for_host)
    message(STATUS "Compiling aui.toolbox for the host platform")
    set(_workdir ${CMAKE_CURRENT_BINARY_DIR}/aui.toolbox_provider)
    file(MAKE_DIRECTORY ${_workdir})
    file(MAKE_DIRECTORY ${_workdir}/b)
    file(WRITE ${_workdir}/CMakeLists.txt [[
cmake_minimum_required(VERSION 3.16)
project(aui.toolbox_provider)
set(CMAKE_CXX_STANDARD 20)
set(BUILD_SHARED_LIBS FALSE)
set(AUI_VERSION v7.1.1)
file(
        DOWNLOAD
        https://raw.githubusercontent.com/aui-framework/aui/${AUI_VERSION}/aui.boot.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)

auib_import(aui https://github.com/aui-framework/aui
            COMPONENTS core toolbox image
            VERSION ${AUI_VERSION})
]])
    set(_build_log ${CMAKE_CURRENT_BINARY_DIR}/aui.toolbox_provider_log.txt)

    # on android build, the host's compiler is overriden by CC and CXX environment variables, which we don't want.
    # we should temporary unset these variables for aui.toolbox build.
    set(_old_CC $ENV{CC})
    set(_old_CXX $ENV{CXX})
    unset(ENV{CC})
    unset(ENV{CXX})

    # /crosscompile-host dir is needed to avoid repo deadlock when crosscompiling
    set(_generator ${CMAKE_GENERATOR})
    if (APPLE)
        # Xcode build requires signing
        set(_generator Ninja)
    endif()
    unset(_forwarded_args)
    foreach(_name AUIB_NO_PRECOMPILED
                  AUIB_FORCE_PRECOMPILED)
        list(APPEND _forwarded_args "-D${_name}=${${_name}}")
    endforeach()
    execute_process(COMMAND ${CMAKE_COMMAND} .. -G${_generator} -DAUIB_CACHE_DIR=${AUIB_CACHE_DIR}/crosscompile-host ${_forwarded_args}
                    WORKING_DIRECTORY ${_workdir}/b
                    RESULT_VARIABLE _r
                    OUTPUT_FILE ${_build_log}
                    ERROR_FILE ${_build_log})

    _auib_dump_with_prefix("[aui.toolbox_provider]" ${_build_log})

    set(ENV{CC} ${_old_CC})
    set(ENV{CXX} ${_old_CXX})

    if (NOT _r STREQUAL 0)
        message(FATAL_ERROR "Unable to build aui.toolbox for the host system (check ${_build_log})")
    endif()
    _aui_try_find_toolbox()
    if (NOT AUI_TOOLBOX_EXE)
        message(FATAL_ERROR "Could not provide aui.toolbox (AUI_TOOLBOX_EXE) - giving up")
    endif()
endmacro()

function(_aui_check_toolbox)
    if (AUI_TOOLBOX_EXE)
        return()
    endif()
    if (CMAKE_CROSSCOMPILING)
        # the worst case because we (possibly) have to compile aui.toolbox for the host system
        # compile aui.toolbox for the host system
        message(STATUS "aui.toolbox for the host system is not found - compiling")
        _aui_provide_toolbox_for_host()
        return()
    endif()
    if (TARGET aui.toolbox)
        set(AUI_TOOLBOX_EXE $<TARGET_FILE:aui.toolbox> CACHE FILEPATH "aui.toolbox" FORCE)
        return()
    endif()
    _aui_try_find_toolbox()
    if (AUI_TOOLBOX_EXE)
        return()
    endif()
    set(AUI_TOOLBOX_EXE ${AUI_DIR}/bin/aui.toolbox CACHE FILEPATH "aui.toolbox" FORCE)
endfunction()

function(aui_compile_assets AUI_MODULE_NAME)
    set(oneValueArgs DIR)
    cmake_parse_arguments(ASSETS "" "${oneValueArgs}" "EXCLUDE" ${ARGN})
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)

    if(CMAKE_CROSSCOMPILING)
        set(TARGET_DIR ${AUI_SDK_BIN})
    else()
        get_target_property(TARGET_DIR ${AUI_MODULE_NAME} ARCHIVE_OUTPUT_DIRECTORY)
    endif()

    if (NOT ASSETS_DIR)
        set(ASSETS_DIR "assets")
    endif()

    get_filename_component(ASSETS_DIR "${ASSETS_DIR}" ABSOLUTE)
    get_filename_component(SELF_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
    get_filename_component(SELF_DIR "${SELF_DIR}" ABSOLUTE)
    if (NOT EXISTS ${ASSETS_DIR})
        message(FATAL_ERROR "aui_compile_assets(${AUI_MODULE_NAME}): expects \"${ASSETS_DIR}\" to exist to compile assets")
    endif()
    file(GLOB_RECURSE ASSETS RELATIVE ${SELF_DIR} "${ASSETS_DIR}/*")

    if (ASSETS_EXCLUDE)
        foreach(_item ${ASSETS})
            string(SUBSTRING ${_item} 7 -1 _path)
            if (_path IN_LIST ASSETS_EXCLUDE)
                list(REMOVE_ITEM ASSETS ${_item})
            endif()
        endforeach()
    endif()

    _aui_check_toolbox()
    message(STATUS "aui.toolbox: using ${AUI_TOOLBOX_EXE} to compile assets for ${AUI_MODULE_NAME}")
    foreach(ASSET_PATH ${ASSETS})
        string(MD5 OUTPUT_PATH ${ASSET_PATH})
        set(OUTPUT_PATH "${CMAKE_CURRENT_BINARY_DIR}/autogen/${OUTPUT_PATH}.cpp")
        get_filename_component(_in "${SELF_DIR}/${ASSET_PATH}" ABSOLUTE)
        get_filename_component(_out "${OUTPUT_PATH}" ABSOLUTE)
        add_custom_command(
                OUTPUT ${OUTPUT_PATH}
                COMMAND ${AUI_TOOLBOX_EXE} pack ${ASSETS_DIR} ${_in} ${_out}
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
        #
        # thus, some libraries (such as aui.views) require wholearchive linking in order to import all
        # statically-initialized variables (like AStylesheet) to the final execution module.
        #
        # also, ordering matters (when it comes to static linking, again).
        # if A depends on B; C depends on both A and B, you should write
        # ```
        # target_link_libraries(C PRIVATE A B)
        # ```
        #
        # and not
        # ```
        # target_link_libraries(C PRIVATE B A)  #Wrong!
        # ```
        #
        # and there's INTERFACE_AUI_WHOLEARCHIVE which specifies how the library should be linked.
        # good luck!
        foreach(_visibility ${multiValueArgs})
            if (_visibility STREQUAL "PRIVATE")
                set(_public_visibility PUBLIC)
            else()
                set(_public_visibility ${_visibility})
            endif()
            foreach(_dep ${AUIL_${_visibility}})
                get_target_property(_already_linked_libs ${AUI_MODULE_NAME} INTERFACE_LINK_LIBRARIES)
                if (${_dep} IN_LIST _already_linked_libs)
                    continue()
                endif()
                # check both :: and .
                string(REPLACE "::" "." _already_linked_libs "${_already_linked_libs}")
                if (${_dep} IN_LIST _already_linked_libs)
                    continue()
                endif()

                # linking library preferring public visibility.
                target_link_libraries(${AUI_MODULE_NAME} ${_public_visibility} ${_dep})
                if (TARGET ${_dep})
                    # adding target's interface include directories and definitions keeping original visibility.
                    get_target_property(_dep_includes ${_dep} INTERFACE_INCLUDE_DIRECTORIES)
                    if (_dep_includes)
                        target_include_directories(${AUI_MODULE_NAME} ${_visibility} $<BUILD_INTERFACE:${_dep_includes}>)
                    endif()

                    get_target_property(_dep_defs ${_dep} INTERFACE_COMPILE_DEFINITIONS)
                    if (_dep_defs)
                        target_compile_definitions(${AUI_MODULE_NAME} ${_visibility} $<BUILD_INTERFACE:${_dep_defs}>)
                    endif()
                endif()
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
    file(GLOB_RECURSE SRCS_TESTS_TMP ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/tests/*.c)


    set(options WHOLEARCHIVE PLUGIN FORCE_STATIC FORCE_SHARED)
    set(oneValueArgs EXPORT)
    set(multiValueArgs ADDITIONAL_SRCS)
    cmake_parse_arguments(AUIE "${options}" "${oneValueArgs}"
            "${multiValueArgs}" ${ARGN} )

    if (AUIE_FORCE_SHARED AND AUIE_FORCE_STATIC)
        message(FATAL_ERROR "FORCE_SHARED AND FORCE_STATIC flags are exclusive!")
    endif()


    file(GLOB_RECURSE SRCS
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.manifest
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.mm
            ${CMAKE_CURRENT_SOURCE_DIR}/src/*.m)

    if (BUILD_SHARED_LIBS)
        if (WIN32)
            if (EXISTS "${CMAKE_SOURCE_DIR}/Resource.rc")
                set(SRCS ${SRCS} "${CMAKE_SOURCE_DIR}/Resource.rc")
            endif()
        endif()
    endif()

    # remove platform dependent files
    foreach(PLATFORM_NAME ${AUI_EXCLUDE_PLATFORMS})
        list(FILTER SRCS EXCLUDE REGEX "(.*\\/)?[Pp]latform/${PLATFORM_NAME}\\/.*")
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

    # AUI.Core -> AUI_CORE
    string(REPLACE "." "_" BUILD_DEF_NAME ${AUI_MODULE_NAME})
    string(TOUPPER "API_${BUILD_DEF_NAME}" BUILD_DEF_NAME)
    target_compile_definitions(${AUI_MODULE_NAME} INTERFACE ${BUILD_DEF_NAME}=AUI_IMPORT)
    target_compile_definitions(${AUI_MODULE_NAME} PRIVATE ${BUILD_DEF_NAME}=AUI_EXPORT)
    target_compile_definitions(${AUI_MODULE_NAME} PUBLIC GLM_FORCE_INLINE=1)

    aui_add_properties(${AUI_MODULE_NAME})
    set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_INTERFACE_MULTIPLICITY 1)

    string(REPLACE "." "::" BUILD_AS_IMPORTED_NAME ${AUI_MODULE_NAME})

    aui_common(${AUI_MODULE_NAME})
    if (AUIE_EXPORT)
        install(
                TARGETS ${AUI_MODULE_NAME}
                EXPORT ${AUIE_EXPORT}
                ARCHIVE       DESTINATION "lib"
                LIBRARY       DESTINATION "lib"
                RUNTIME       DESTINATION "bin"
                PUBLIC_HEADER DESTINATION "${AUI_MODULE_NAME}/include"
                INCLUDES      DESTINATION "${AUI_MODULE_NAME}/include"
        )

        install(
                DIRECTORY src/
                DESTINATION "${AUI_MODULE_NAME}/include/"
                FILES_MATCHING PATTERN "*.h"
                PATTERN "*.hpp"

        )
        target_include_directories(${AUI_MODULE_NAME} INTERFACE $<INSTALL_INTERFACE:${AUI_MODULE_NAME}/include/>)
    endif()
    if (NOT BUILD_AS_IMPORTED_NAME STREQUAL ${AUI_MODULE_NAME})
        add_library(${BUILD_AS_IMPORTED_NAME} ALIAS ${AUI_MODULE_NAME})
    endif()

    if (AUIE_COMPILE_ASSETS)
        aui_compile_assets(${AUI_MODULE_NAME})
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)
    endif()

    if (AUIE_PLUGIN OR AUIE_WHOLEARCHIVE)
        set_target_properties(${AUI_MODULE_NAME} PROPERTIES INTERFACE_AUI_WHOLEARCHIVE ON)

        if (AUIE_PLUGIN)
            # define plugin entry for plugins
            if (BUILD_SHARED_LIBS)
                target_compile_definitions(${AUI_MODULE_NAME} PRIVATE _AUI_PLUGIN_ENTRY_N=aui_plugin_entry)
            else()
                target_compile_definitions(${AUI_MODULE_NAME} PRIVATE _AUI_PLUGIN_ENTRY_N=aui_plugin_entry_${BUILD_DEF_NAME})
            endif()
        endif()
    endif()

    get_target_property(_type ${AUI_MODULE_NAME} TYPE)
    if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
        include(WinXPuseLTL5)
        include(WinXPuseYYThunks)
        target_link_libraries(${AUI_MODULE_NAME} PRIVATE YY_Thunks)
        if (CMAKE_SIZEOF_VOID_P EQUAL 4)
            if(_type STREQUAL "SHARED_LIBRARY")
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:DllMainCRTStartupForYY_Thunks")
            elseif(_type STREQUAL "STATIC_LIBRARY")
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
            elseif(_type STREQUAL "EXECUTABLE") # Assume toolbox and other AUI components are console
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01")
            else()
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01")
            endif()
        else()
            if(_type STREQUAL "SHARED_LIBRARY")
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.02 /SUBSYSTEM:WINDOWS,5.02 /ENTRY:DllMainCRTStartupForYY_Thunks")
            elseif(_type STREQUAL "STATIC_LIBRARY")
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.02 /SUBSYSTEM:WINDOWS,5.02")
            elseif(_type STREQUAL "EXECUTABLE") # Assume toolbox and other AUI components are console
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.02")
            else()
                set_target_properties(${AUI_MODULE_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE,5.02 /SUBSYSTEM:WINDOWS,5.02")
            endif()
        endif()
    endif()
    if (_type STREQUAL "STATIC_LIBRARY")
        if (MSVC)
            target_link_options(${AUI_MODULE_NAME} PUBLIC "$<$<BOOL:$<TARGET_PROPERTY:${AUI_MODULE_NAME},INTERFACE_AUI_WHOLEARCHIVE>>:/WHOLEARCHIVE:$<TARGET_FILE:${AUI_MODULE_NAME}>>")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            if (APPLE)
                target_link_options(${AUI_MODULE_NAME} PUBLIC "$<$<BOOL:$<TARGET_PROPERTY:${AUI_MODULE_NAME},INTERFACE_AUI_WHOLEARCHIVE>>:-Wl,-force_load,$<TARGET_FILE:${AUI_MODULE_NAME}>>")
            else()
                target_link_options(${AUI_MODULE_NAME} PUBLIC "$<$<BOOL:$<TARGET_PROPERTY:${AUI_MODULE_NAME},INTERFACE_AUI_WHOLEARCHIVE>>:-Wl,--whole-archive,--allow-multiple-definition,$<TARGET_FILE:${AUI_MODULE_NAME}>,--no-whole-archive>")
            endif()
        endif()
    endif()
    if (CMAKE_CXX_COMPILER_ID MATCHES "AppleClang")
        add_custom_command(
            TARGET ${AUI_MODULE_NAME}
            POST_BUILD
            COMMAND $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:${CMAKE_STRIP}>
            ARGS    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-x>
					$<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-S>
                    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:$<TARGET_FILE:${AUI_MODULE_NAME}>>
            COMMENT "Stripping ${AUI_MODULE_NAME} (only for Release/MinSizeRel)"
            VERBATIM
        )
    elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        if(APPLE)
            add_custom_command(
                TARGET ${AUI_MODULE_NAME}
                POST_BUILD
                COMMAND $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:${CMAKE_STRIP}>
                ARGS    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-Sxl>
                        $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:$<TARGET_FILE:${AUI_MODULE_NAME}>>
                COMMENT "Stripping ${AUI_MODULE_NAME} (only for Release/MinSizeRel)"
                VERBATIM
            )
        else()
            add_custom_command(
                TARGET ${AUI_MODULE_NAME}
                POST_BUILD
                COMMAND $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:${CMAKE_STRIP}>
                ARGS    $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-g>
                        $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:-x>
                        $<$<OR:$<CONFIG:Release>,$<CONFIG:MinSizeRel>>:$<TARGET_FILE:${AUI_MODULE_NAME}>>
                COMMENT "Stripping ${AUI_MODULE_NAME} (only for Release/MinSizeRel)"
                VERBATIM
            )
        endif()
    endif()
endfunction(aui_module)

# links the auisl shader located in shaders/<NAME>
function(auisl_shader TARGET NAME)
    set(_path ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${NAME})
    if (NOT EXISTS ${_path})
        message(FATAL_ERROR "shader not exists: ${_path}")
    endif()

    if (NOT TARGET ${TARGET})
        message(FATAL_ERROR "no such target: ${TARGET}")
    endif()

    set(_compiled_shader_dir ${CMAKE_CURRENT_BINARY_DIR}/shaders/AUISL/Generated)
    file(MAKE_DIRECTORY ${_compiled_shader_dir})

    set(_targets software glsl120)
    _aui_check_toolbox()
    foreach(_target ${_targets})
        set(_output "${_compiled_shader_dir}/${NAME}.${_target}.cpp")

        add_custom_command(
                OUTPUT ${_output}
                DEPENDS ${_path}
                COMMAND ${AUI_TOOLBOX_EXE}
                ARGS auisl ${_target} ${_path} ${_output}
        )
        target_sources(${TARGET} PRIVATE ${_output})
    endforeach()

    target_include_directories(${TARGET} PRIVATE ${CMAKE_CURRENT_BINARY_DIR}/shaders)
endfunction()

macro(_auib_weak_set VAR_NAME)
    if (NOT VAR_NAME)
        set(${ARGV})
    endif()
endmacro()

function(_auib_weak_set_target_property TARGET PROPERTY VALUE)
    get_target_property(_tmp ${TARGET} ${PROPERTY})
    if (NOT _tmp)
        set_target_properties(${TARGET} PROPERTIES ${PROPERTY} "${VALUE}")
    endif()
endfunction()

function(aui_set_cpack_generator GENERATOR)
    set(CPACK_GENERATOR ${GENERATOR} CACHE STRING "CPack Generator (managed by AUI_APP_PACKAGING)" FORCE)
    message(STATUS "CPACK_GENERATOR=${GENERATOR} (managed by AUI_APP_PACKAGING)")
endfunction()

function(aui_get_package_name_and_arch _out)
    set(_system_name "${CMAKE_SYSTEM_NAME}")
    if (_system_name MATCHES "[Dd]arwin")
        set(_system_name "macos") # darwin is not user friendly name, using macos instead
    endif()
    string(TOLOWER "${_system_name}-${CMAKE_SYSTEM_PROCESSOR}" _tmp)
    set(${_out} ${_tmp} PARENT_SCOPE)
endfunction()

macro(aui_app)
    _aui_find_root()

    set(options NO_INCLUDE_CPACK)
    set(oneValueArgs
            # common
            TARGET
            NAME
            COPYRIGHT
            VERSION
            ICON
            VENDOR

            # android
            ANDROID_PACKAGE

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

    file(WRITE "${CMAKE_BINARY_DIR}/appinfo.cpp" "#include <AUI/AppInfo.h>
    struct AUIAppInfo {
        AUIAppInfo() {
            aui::app_info::name = \"${APP_NAME}\";
        }
    }; AUIAppInfo auiAppInfo;")

    target_sources(${APP_TARGET} PUBLIC ${CMAKE_BINARY_DIR}/appinfo.cpp)

    # defaults
    # ios
    if (NOT APP_IOS_VERSION)
        set(APP_IOS_VERSION 14.3)
    endif()
    if (NOT APP_IOS_DEVICE)
        set(APP_IOS_DEVICE BOTH)
    endif()
    if (NOT APP_VERSION)
        if (PROJECT_VERSION)
            set(APP_VERSION ${PROJECT_VERSION})
        else()
            set(APP_VERSION 1.0)
        endif()
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
    if (NOT APP_TARGET)
        if (NOT TARGET ${APP_TARGET})
            list(APPEND _error_msg "TARGET which is your app's executable target.")
        endif()
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
        list(APPEND _error_msg_opt "IOS_CONTROLLER which is your controller name (defaults to AUIViewController).")
    endif()

    if (AUI_PLATFORM_LINUX)
        if (NOT APP_LINUX_DESKTOP_FILE)
            list(APPEND _error_msg_opt "LINUX_DESKTOP_FILE which is your custom *.desktop file.")
        endif()
    endif()

    if (AUI_PLATFORM_ANDROID OR AUI_BUILD_FOR STREQUAL "android")
        if (NOT APP_ANDROID_PACKAGE)
            list(APPEND _error_msg "ANDROID_PACKAGE which is android app package name.")
        endif()
    endif()

    if (_error_msg)
        list(JOIN _error_msg "\n - " v1)
        list(JOIN _error_msg_opt "\n - " v2)
        foreach(_i v1 v2)
            if (${_i})
                set(${_i} " - ${${_i}}")
            endif()
        endforeach()
        message(FATAL_ERROR "The following arguments are required for aui_app():\n${v1}\nnote: the following optional variables can be also set:\n${v2}")
    endif()

    set(_current_app_build_files ${PROJECT_BINARY_DIR}/${APP_TARGET})
    if (APP_ICON)
        get_filename_component(_icon_absolute ${APP_ICON} ABSOLUTE)
    endif()


    # common cpack
    _auib_weak_set(CPACK_PACKAGE_NAME ${APP_NAME})
    _auib_weak_set(CPACK_BUNDLE_NAME ${APP_NAME})
    _auib_weak_set(CPACK_PACKAGE_VENDOR ${APP_VENDOR})
    _auib_weak_set(CPACK_PACKAGE_VERSION ${APP_VERSION})
    _auib_weak_set(CPACK_BUNDLE_PLIST ${_current_app_build_files}/MacOSXBundleInfo.plist)

    file(WRITE ${_current_app_build_files}/copyright.txt ${APP_COPYRIGHT})
    aui_get_package_name_and_arch(_aui_package_file_name)
    set(_aui_package_file_name "${CPACK_PACKAGE_NAME}-${APP_VERSION}-${_aui_package_file_name}")

    if ("${AUI_APP_PACKAGING}" MATCHES "^AUI_.*")
        _aui_find_root()
        string(SUBSTRING "${AUI_APP_PACKAGING}" 4 -1 _indirection)
        string(TOLOWER "${_indirection}" _indirection)
        set(_indirection "cmake/aui.app_packaging.${_indirection}.cmake")
        set(_ok FALSE)
        set(_tried_paths "")
        foreach (_probe_dir "${CMAKE_CURRENT_LIST_DIR}" "${AUI_BUILD_AUI_ROOT}")
            set(_indirection_full "${_probe_dir}/${_indirection}")
            if (EXISTS ${_indirection_full})
                message(STATUS "AUI_APP_PACKAGING uses ${_indirection_full}")
                include(${_indirection_full})
                set(_ok TRUE)
                break()
            else()
                set(_tried_paths "${_tried_paths} ${_indirection_full}")
            endif()
        endforeach ()
        if (NOT ${_ok})
            message(FATAL_ERROR "Unknown packaging method ${AUI_APP_PACKAGING}! Tried paths:" ${_tried_paths})
        endif()
    elseif(AUI_APP_PACKAGING)
        aui_set_cpack_generator(${AUI_APP_PACKAGING})
    endif ()

    # WINDOWS ==========================================================================================================
    if (AUI_PLATFORM_WIN)
        set_target_properties(${APP_TARGET} PROPERTIES AUI_INSTALL_RUNTIME_DIR ".")
        install(TARGETS ${APP_TARGET}
                DESTINATION "$<TARGET_PROPERTY:${APP_TARGET},AUI_INSTALL_RUNTIME_DIR>")

        _auib_weak_set(CPACK_RESOURCE_FILE_LICENSE ${_current_app_build_files}/copyright.txt) # windows only APP_COPYRIGHT
        get_target_property(_executable ${APP_TARGET} OUTPUT_NAME)
        _auib_weak_set(CPACK_PACKAGE_EXECUTABLES "${_executable};${APP_NAME}") # windows only
        _auib_weak_set(CPACK_CREATE_DESKTOP_LINKS "${_executable}") # windows only
        if ("INNOSETUP" IN_LIST CPACK_GENERATOR)
            set(_aui_package_file_name ${_aui_package_file_name}-setup) # append -setup suffix for INNOSETUP
        endif()
        if (APP_ICON)
            set(_ico "${_current_app_build_files}/app.ico")
            add_custom_command(
                    OUTPUT ${_ico}
                    COMMAND ${AUI_TOOLBOX_EXE}
                    ARGS svg2ico ${_icon_absolute} ${_ico}
            )
            configure_file(${AUI_BUILD_AUI_ROOT}/platform/win32/res.rc.in ${_current_app_build_files}/win32-res.rc)
            target_sources(${APP_TARGET} PRIVATE ${_current_app_build_files}/win32-res.rc ${_ico})
            _auib_weak_set(CPACK_INNOSETUP_ICON_FILE ${_ico}) # installer icon
            _auib_weak_set(CPACK_WIX_PRODUCT_ICON ${_ico}) # displays app icon in Control Panel/Settings

            if ("WIX" IN_LIST CPACK_GENERATOR)
                set(_ico "${_current_app_build_files}/wix_ui_banner.bmp")
                add_custom_command(
                        OUTPUT ${_ico}
                        COMMAND ${AUI_TOOLBOX_EXE}
                        ARGS convert-image ${_icon_absolute} ${_ico} -p=435x0 -c=493x58
                )
                target_sources(${APP_TARGET} PRIVATE ${_ico})
                _auib_weak_set(CPACK_WIX_UI_BANNER ${_ico}) # image at top of all installer pages

                set(_ico "${_current_app_build_files}/wix_ui_dialog.bmp")
                add_custom_command(
                        OUTPUT ${_ico}
                        COMMAND ${AUI_TOOLBOX_EXE}
                        ARGS convert-image ${_icon_absolute} ${_ico} -p=0x0 -c=493x312 -r=170
                )
                target_sources(${APP_TARGET} PRIVATE ${_ico})
                _auib_weak_set(CPACK_WIX_UI_DIALOG ${_ico}) # background image used on the welcome and completion dialogs
            endif()
            if ("INNOSETUP" IN_LIST CPACK_GENERATOR)
                set(_ico "${_current_app_build_files}/innosetup.bmp")
                add_custom_command(
                        OUTPUT ${_ico}
                        COMMAND ${AUI_TOOLBOX_EXE}
                        ARGS convert-image ${_icon_absolute} ${_ico} -c=256 -b=\#ffffff
                )
                target_sources(${APP_TARGET} PRIVATE ${_ico})
                _auib_weak_set(CPACK_PACKAGE_ICON ${_ico}) # app icon inside INNOSETUP window
                _auib_weak_set(CPACK_INNOSETUP_SETUP_WizardSmallImageFile ${_ico}) # small icon inside INNOSETUP window
                _auib_weak_set(CPACK_INNOSETUP_SETUP_UninstallDisplayIcon "{app}\\\\\\\\bin\\\\\\\\${_executable}.exe") # displays app icon in Control Panel/Settings
                _auib_weak_set(CPACK_INNOSETUP_SETUP_PrivilegesRequired "lowest") # hence we're installing to user dir, we don't need UAC
                _auib_weak_set(CPACK_INNOSETUP_IGNORE_LICENSE_PAGE ON) # skips license page
                _auib_weak_set(CPACK_INNOSETUP_IGNORE_README_PAGE ON) # skips README page
            endif()
        endif()
        set_property(INSTALL bin/$<TARGET_FILE_NAME:${APP_TARGET}> PROPERTY CPACK_START_MENU_SHORTCUTS "${APP_NAME}")
        set_property(INSTALL bin/$<TARGET_FILE_NAME:${APP_TARGET}> PROPERTY CPACK_DESKTOP_SHORTCUTS "${APP_NAME}")
        _auib_weak_set(CPACK_PACKAGE_INSTALL_DIRECTORY ${APP_NAME}) # removes -VERSION suffix
        _auib_weak_set(CPACK_WIX_PROGRAM_MENU_FOLDER ".") # omits Start menu folder
        _auib_weak_set(CPACK_INNOSETUP_PROGRAM_MENU_FOLDER ".") # omits Start menu folder
        _auib_weak_set(CPACK_INNOSETUP_INSTALL_ROOT "{localappdata}") # installs To AppData\\Local
        _auib_weak_set(CPACK_INNOSETUP_RUN_EXECUTABLES ${_executable}) # runs the program after installation
        _auib_weak_set_target_property(${APP_TARGET} CPACK_DESKTOP_SHORTCUTS "${APP_NAME}")
    endif()


    # DESKTOP LINUX ====================================================================================================
    if (AUI_PLATFORM_LINUX)
        install(TARGETS ${APP_TARGET}
                DESTINATION $<TARGET_PROPERTY:${APP_TARGET},AUI_INSTALL_RUNTIME_DIR>)
        get_target_property(_executable ${APP_TARGET} OUTPUT_NAME)
        if (NOT APP_LINUX_DESKTOP)
            # generate desktop file
            set(_desktop "[Desktop Entry]\nName=${APP_NAME}\nExec=${_executable}\nType=Application\nTerminal=false\nCategories=Utility")
            if (APP_ICON)
                set(_icon "${_current_app_build_files}/app.icon.svg")
                configure_file(${APP_ICON} "${_icon}" COPYONLY)
                set(APP_ICON ${_icon})
                set(_desktop "${_desktop}\nIcon=app.icon")
            endif()
            file(GENERATE
                    OUTPUT "${_current_app_build_files}/app.desktop"
                    CONTENT ${_desktop})
            set(APP_LINUX_DESKTOP ${_current_app_build_files}/app.desktop)
        endif()
#        file(GENERATE
#                OUTPUT ${_current_app_build_files}/appimage-generate.cmake
#                INPUT ${AUI_BUILD_AUI_ROOT}/cmake/appimage-generate.cmake.in)
#
#        file(GENERATE
#                OUTPUT ${_current_app_build_files}/appimage-generate-vars.cmake
#                CONTENT "set(EXECUTABLE $<TARGET_FILE:${APP_TARGET}>)\nset(DESKTOP_FILE ${APP_LINUX_DESKTOP})\nset(ICON_FILE ${APP_ICON})")
#        set(APP_LINUX_DESKTOP ${_current_app_build_files}/appimage-generate.cmake)
#
#        list(APPEND CPACK_GENERATOR External)
#
#        set(CPACK_EXTERNAL_PACKAGE_SCRIPT "${_current_app_build_files}/appimage-generate.cmake")
#        set(CPACK_EXTERNAL_ENABLE_STAGING YES)
    endif()

    # IOS AND MACOS ====================================================================================================
    if (APPLE)
        string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" _cmake_system_processor_lower)
        _auib_weak_set(CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${APP_VERSION}-${_cmake_system_processor_lower}) # proper system processor on macOS
        if (NOT APP_APPLE_BUNDLE_IDENTIFIER)
            set(APP_APPLE_BUNDLE_IDENTIFIER ${APP_NAME})
        endif()
        set(PRODUCT_NAME ${APP_NAME})
        set(EXECUTABLE_NAME ${APP_NAME})
        _auib_weak_set(MACOSX_BUNDLE_EXECUTABLE_NAME ${APP_NAME})
        _auib_weak_set(MACOSX_BUNDLE_INFO_STRING ${APP_APPLE_BUNDLE_IDENTIFIER})
        _auib_weak_set(MACOSX_BUNDLE_GUI_IDENTIFIER ${APP_APPLE_BUNDLE_IDENTIFIER})
        _auib_weak_set(MACOSX_BUNDLE_BUNDLE_NAME ${APP_NAME})
        _auib_weak_set(MACOSX_BUNDLE_ICON_FILE "app.icns")
        _auib_weak_set(MACOSX_BUNDLE_LONG_VERSION_STRING ${APP_VERSION})
        _auib_weak_set(MACOSX_BUNDLE_SHORT_VERSION_STRING ${APP_VERSION})
        _auib_weak_set(MACOSX_BUNDLE_BUNDLE_VERSION ${APP_VERSION})
        _auib_weak_set(MACOSX_BUNDLE_COPYRIGHT ${APP_COPYRIGHT})
        _auib_weak_set(MACOSX_DEPLOYMENT_TARGET ${APP_IOS_VERSION})
        if (AUI_PLATFORM_MACOS)
            configure_file(${AUI_BUILD_AUI_ROOT}/platform/apple/bundleinfo.plist.in ${CPACK_BUNDLE_PLIST})
        endif()
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE TRUE)
        _auib_weak_set_target_property(${APP_TARGET} BUNDLE TRUE)
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_INFO_PLIST           "${CPACK_BUNDLE_PLIST}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_EXECUTABLE_NAME      "${MACOSX_BUNDLE_EXECUTABLE_NAME}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_INFO_STRING          "${MACOSX_BUNDLE_INFO_STRING}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_GUI_IDENTIFIER       "${MACOSX_BUNDLE_GUI_IDENTIFIER}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_BUNDLE_NAME          "${MACOSX_BUNDLE_BUNDLE_NAME}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_ICON_FILE            "${MACOSX_BUNDLE_ICON_FILE}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_LONG_VERSION_STRING  "${MACOSX_BUNDLE_LONG_VERSION_STRING}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_SHORT_VERSION_STRING "${MACOSX_BUNDLE_SHORT_VERSION_STRING}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_BUNDLE_VERSION       "${MACOSX_BUNDLE_BUNDLE_VERSION}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_BUNDLE_COPYRIGHT            "${MACOSX_BUNDLE_COPYRIGHT}")
        _auib_weak_set_target_property(${APP_TARGET} MACOSX_DEPLOYMENT_TARGET           "${MACOSX_DEPLOYMENT_TARGET}")

        set_target_properties(${APP_TARGET} PROPERTIES OUTPUT_NAME "${APP_NAME}") # rename the bundle to display name
    endif()

    # MACOS ============================================================================================================
    if (AUI_PLATFORM_MACOS)
        configure_file(${AUI_BUILD_AUI_ROOT}/platform/apple/bundleinfo.plist.in ${CPACK_BUNDLE_PLIST})

        if (APP_ICON)
            # generate icns
            set(_icons_dir ${_current_app_build_files}/app.iconset)
            set(_resolutions 16 32 64 128 256 512 1024)

            unset(_outputs)
            foreach(_res ${_resolutions})
                list(APPEND _outputs "${_icons_dir}/icon_${_res}x${_res}.png")
            endforeach()
            list(JOIN _resolutions , _resolutions_comma)
            if (TARGET aui.toolbox)
                add_dependencies(${APP_TARGET} aui.toolbox)
            endif()
            set(_icon_icns ${_current_app_build_files}/app.icns)
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


        configure_file(${AUI_BUILD_AUI_ROOT}/platform/apple/dmg_background.png ${_current_app_build_files}/dmg_background.png COPYONLY)
        _auib_weak_set(CPACK_DMG_BACKGROUND_IMAGE ${_current_app_build_files}/dmg_background.png) # sets the default DMG background

        configure_file(${AUI_BUILD_AUI_ROOT}/platform/apple/dmg_ds_store_setup.scpt ${_current_app_build_files}/dmg_ds_store_setup.scpt)
        _auib_weak_set(CPACK_DMG_DS_STORE_SETUP_SCRIPT ${_current_app_build_files}/dmg_ds_store_setup.scpt) # rearranges icons in DMG

        install(TARGETS ${APP_TARGET} BUNDLE DESTINATION ".")
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
                ${CMAKE_CURRENT_BINARY_DIR}/LaunchScreen.storyboard
        )

        configure_file(${AUI_BUILD_AUI_ROOT}/platform/ios/LaunchScreen.storyboard.in ${CMAKE_CURRENT_BINARY_DIR}/LaunchScreen.storyboard @ONLY)

        target_sources(${APP_TARGET} PRIVATE ${RESOURCES})
        set_target_properties(${APP_TARGET} PROPERTIES XCODE_ATTRIBUTE_ENABLE_BITCODE "NO")

        # Locate system libraries on iOS
        find_library(UIKIT UIKit REQUIRED)
        find_library(COREANIMATION QuartzCore REQUIRED)
        find_library(OPENGL OpenGLES REQUIRED)
        find_library(FOUNDATION Foundation REQUIRED)
        find_library(MOBILECORESERVICES MobileCoreServices REQUIRED)
        find_library(CFNETWORK CFNetwork REQUIRED)
        find_library(SYSTEMCONFIGURATION SystemConfiguration REQUIRED)

        # link the frameworks located above
        target_link_libraries(${APP_TARGET}
                PRIVATE
                ${OPENGL}
                ${UIKIT}
                ${FOUNDATION}
                ${MOBILECORESERVICES}
                ${CFNETWORK}
                ${COREANIMATION}
                ${SYSTEMCONFIGURATION})


        # Turn on ARC
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc")

        if (NOT DEFINED AUI_IOS_CODE_SIGNING_REQUIRED)
            set(AUI_IOS_CODE_SIGNING_REQUIRED YES)
        endif()

        # Create the app target
        set_target_properties(${APP_TARGET} PROPERTIES
                XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT "dwarf-with-dsym"
                RESOURCE "${RESOURCES}"
                XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "YES"
                XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${APP_IOS_VERSION}
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY ${APP_APPLE_SIGN_IDENTITY}
                XCODE_ATTRIBUTE_DEVELOPMENT_TEAM ${APP_APPLE_TEAM_ID}
                XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY ${APP_IOS_DEVICE}
                MACOSX_BUNDLE_INFO_PLIST ${AUI_BUILD_AUI_ROOT}/platform/apple/plist.in
                XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
                XCODE_ATTRIBUTE_COMBINE_HIDPI_IMAGES NO
                XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
                XCODE_ATTRIBUTE_ENABLE_TESTABILITY YES
                XCODE_ATTRIBUTE_GCC_SYMBOLS_PRIVATE_EXTERN YES
                XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED ${AUI_IOS_CODE_SIGNING_REQUIRED}
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
                ${_current_app_build_files}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks
                \&\>/dev/null \; then
                COMMAND_DONE=1 \;
                fi \;
                if ${CMAKE_COMMAND} -E make_directory
                \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks
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
                ${_current_app_build_files}/cppframework/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/
                ${_current_app_build_files}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks
                \&\>/dev/null \; then
                COMMAND_DONE=1 \;
                fi \;
                if ${CMAKE_COMMAND} -E copy_directory
                \${BUILT_PRODUCTS_DIR}/${FRAMEWORK_NAME}.framework
                \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
                \&\>/dev/null \; then
                COMMAND_DONE=1 \;
                fi \;
                if [ \\$$COMMAND_DONE -eq 0 ] \; then
                echo Failed to copy the framework into the app bundle \;
                exit 1 \;
                fi\"
        )
        # Codesign the framework in it's new spot
        if (AUI_IOS_CODE_SIGNING_REQUIRED)
            add_custom_command(
                    TARGET
                    ${APP_TARGET}
                    POST_BUILD COMMAND /bin/sh -c
                    \"COMMAND_DONE=0 \;
                    if codesign --force --verbose
                    ${_current_app_build_files}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
                    --sign ${APP_APPLE_SIGN_IDENTITY}
                    \&\>/dev/null \; then
                    COMMAND_DONE=1 \;
                    fi \;
                    if codesign --force --verbose
                    \${BUILT_PRODUCTS_DIR}/${APP_NAME}.app/Frameworks/${FRAMEWORK_NAME}.framework
                    --sign ${APP_APPLE_SIGN_IDENTITY}
                    \&\>/dev/null \; then
                    COMMAND_DONE=1 \;
                    fi \;
                    if [ \\$$COMMAND_DONE -eq 0 ] \; then
                    echo Framework codesign failed \;
                    exit 1 \;
                    fi\"
            )

        endif()

        # Add a "PlugIns" folder as a kludge fix for how the XcTest package generates paths
        add_custom_command(
                TARGET
                ${APP_TARGET}
                POST_BUILD COMMAND /bin/sh -c
                \"COMMAND_DONE=0 \;
                if ${CMAKE_COMMAND} -E make_directory
                ${_current_app_build_files}/\${CONFIGURATION}\${EFFECTIVE_PLATFORM_NAME}/PlugIns
                \&\>/dev/null \; then
                COMMAND_DONE=1 \;
                fi \;
                if [ \\$$COMMAND_DONE -eq 0 ] \; then
                echo Failed to create PlugIns directory in EFFECTIVE_PLATFORM_NAME folder. \;
                exit 1 \;
                fi\"
        )
    endif()
    string(TOLOWER "${_aui_package_file_name}" _aui_package_file_name)
    string(REPLACE " " "_" _aui_package_file_name "${_aui_package_file_name}")
    _auib_weak_set(CPACK_PACKAGE_FILE_NAME "${_aui_package_file_name}")
    if (NOT APP_NO_INCLUDE_CPACK AND NOT CPack_CMake_INCLUDED)
        include(CPack)
    endif()

    if (AUI_BUILD_FOR STREQUAL "android")
        _aui_android_app()
    elseif (AUI_BUILD_FOR STREQUAL "ios")
        _aui_ios_app()
    endif()
endmacro()

if (MINGW OR UNIX)
    # strip for release
    if (NOT(CMAKE_CXX_COMPILER_ID MATCHES "(AppleClang|Clang)"))
	    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
	    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
        set(CMAKE_C_FLAGS_MINSIZEREL "${CMAKE_C_FLAGS_MINSIZEREL} -s")
        set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s")
    endif()
endif()


# Coverage support
#if(CMAKE_BUILD_TYPE STREQUAL "Debug")
#    if (UNIX)
#        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
#        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
#    endif()
#endif()

if (AUI_BUILD_FOR STREQUAL "winxp")
    # [cmake] -> YY_THUNKS MSVC .obj -> cmake(x86|x64)
    _aui_find_root()
    include(${AUI_BUILD_AUI_ROOT}/cmake/aui.build.winxp.cmake)
endif()

if (AUI_BUILD_FOR STREQUAL "android")
    # [cmake] -> gradle -> cmake(x86|mips|arm|arm64)
    _aui_find_root()
    include(${AUI_BUILD_AUI_ROOT}/cmake/aui.build.android.cmake)
endif()

if (ANDROID)
    # cmake -> gradle -> [cmake(x86|mips|arm|arm64)]
    auib_use_system_libs_begin()
    find_library(log-lib log)
    add_library(aui::android_log IMPORTED SHARED)
    set_target_properties(aui::android_log PROPERTIES
            IMPORTED_LOCATION "${log-lib}")
    auib_use_system_libs_end()
endif()

if (AUI_BUILD_FOR STREQUAL "ios")
    _aui_find_root()
    include(${AUI_BUILD_AUI_ROOT}/cmake/aui.build.ios.cmake)
endif()
