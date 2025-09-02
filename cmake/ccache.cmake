# cmake/EnableCcache.cmake
#
# Usage:
#   enable_ccache(
#       PREFIX <prefix>             # e.g. AUI or AUIB
#       OPTION_DESC <description>   # e.g. "Enable usage of ccache if available"
#       DEFAULT <ON|OFF>            # default value for the option
#   )
#
# This will:
#   - Define <PREFIX>_USE_CCACHE and <PREFIX>_CCACHE_PROGRAM cache vars
#   - Try to find ccache if enabled
#   - Set CMAKE_*_COMPILER_LAUNCHER if not already set

function(enable_ccache)
    set(options)
    set(oneValueArgs PREFIX OPTION_DESC DEFAULT)
    cmake_parse_arguments(EC "${options}" "${oneValueArgs}" "" ${ARGN})

    if(NOT EC_PREFIX)
        message(FATAL_ERROR "enable_ccache: PREFIX is required")
    endif()

    # Define the option and program path cache variable
    option(${EC_PREFIX}_USE_CCACHE "${EC_OPTION_DESC}" ${EC_DEFAULT})
    set(${EC_PREFIX}_CCACHE_PROGRAM "" CACHE PATH
        "${EC_PREFIX}: Path to ccache wrapper or executable (overrides auto-detection)")

    if(${EC_PREFIX}_USE_CCACHE)
        if(NOT ${EC_PREFIX}_CCACHE_PROGRAM)
            find_program(_${EC_PREFIX}_CCACHE_PROGRAM NAMES ccache ccache.exe ccache.bat)
            if(_${EC_PREFIX}_CCACHE_PROGRAM)
                set(${EC_PREFIX}_CCACHE_PROGRAM "${_${EC_PREFIX}_CCACHE_PROGRAM}" CACHE PATH
                    "${EC_PREFIX}: Path to ccache wrapper or executable (overrides auto-detection)" FORCE)
            endif()
        endif()

        if(${EC_PREFIX}_CCACHE_PROGRAM)
            message(STATUS "${EC_PREFIX}: ccache found: ${${EC_PREFIX}_CCACHE_PROGRAM}; enabling compiler launchers")
            if(NOT DEFINED CMAKE_C_COMPILER_LAUNCHER OR CMAKE_C_COMPILER_LAUNCHER STREQUAL "")
                set(CMAKE_C_COMPILER_LAUNCHER ${${EC_PREFIX}_CCACHE_PROGRAM} CACHE STRING "C compiler launcher" FORCE)
            endif()
            if(NOT DEFINED CMAKE_CXX_COMPILER_LAUNCHER OR CMAKE_CXX_COMPILER_LAUNCHER STREQUAL "")
                set(CMAKE_CXX_COMPILER_LAUNCHER ${${EC_PREFIX}_CCACHE_PROGRAM} CACHE STRING "C++ compiler launcher" FORCE)
            endif()
        else()
            message(STATUS "${EC_PREFIX}: ${EC_PREFIX}_USE_CCACHE is ON but ccache was not found in PATH and ${EC_PREFIX}_CCACHE_PROGRAM not set")
        endif()
    endif()
endfunction()
