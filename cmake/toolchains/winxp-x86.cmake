# Windows XP MSVC Toolchain
# Supports Visual Studio 2017 (v141) and later with XP compatibility

set(CMAKE_SYSTEM_NAME       Windows)
set(CMAKE_SYSTEM_VERSION    5.1)
set(CMAKE_SYSTEM_PROCESSOR  x86)

set(CMAKE_C_COMPILER        "cl.exe")
set(CMAKE_CXX_COMPILER      "cl.exe")

set(CMAKE_C_FLAGS   "${CMAKE_C_FLAGS} /D_WIN32_WINNT=0x0501 /DWINVER=0x0501 /EHsc"   CACHE STRING "c flags"     FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_WIN32_WINNT=0x0501 /DWINVER=0x0501 /EHsc" CACHE STRING "c++ flags"   FORCE)

set(CMAKE_GENERATOR_PLATFORM    "Win32" CACHE STRING "Platform"                     FORCE)

set(CMAKE_GENERATOR_TOOLSET     "v141_xp" CACHE STRING "CMAKE_GENERATOR_TOOLSET"        FORCE)
set(CMAKE_VS_PLATFORM_TOOLSET   "v141_xp" CACHE STRING "CMAKE_VS_PLATFORM_TOOLSET"      FORCE)
set(CMAKE_VS_WINDOWS_TARGET_PLATFORM_VERSION    "10.0.26100.0" CACHE STRING "SDK"                FORCE)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "" FORCE)

include(FetchContent)

if(NOT(TARGET YY_Thunks))
    if(NOT DEFINED yy_thunks_SOURCE_DIR)
        set(YY_Thunks_VERSION v1.1.7)
        FetchContent_Declare(yy_thunks
            URL https://github.com/Chuyu-Team/YY-Thunks/releases/download/${YY_Thunks_VERSION}/YY-Thunks-Objs.zip
            DOWNLOAD_EXTRACT_TIMESTAMP true
        )
        FetchContent_MakeAvailable(yy_thunks)
    endif()

    add_library(YY_Thunks INTERFACE)

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(YY_Thunks INTERFACE
            "${yy_thunks_SOURCE_DIR}/objs/x64/YY_Thunks_for_WinXP.obj")
    else()
        target_link_libraries(YY_Thunks INTERFACE
            "${yy_thunks_SOURCE_DIR}/objs/x86/YY_Thunks_for_WinXP.obj")
    endif()

    install(TARGETS YY_Thunks
        EXPORT YY_ThunksTarget
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
    )

    install(EXPORT YY_ThunksTarget
        FILE YY_ThunksTarget.cmake
        NAMESPACE YY_Thunks::
        DESTINATION lib/cmake/YY_Thunks
    )
endif()

set(VC_LTL_VERSION v5.2.2)
set(WindowsTargetPlatformMinVersion "5.1.2600.0")
set(SupportLTL                      "ucrt")
set(LTLPlatform                     "Win32")

FetchContent_Declare(vc_ltl 
    URL https://github.com/Chuyu-Team/VC-LTL5/releases/download/${VC_LTL_VERSION}/VC-LTL-Binary.7z
    DOWNLOAD_EXTRACT_TIMESTAMP true
)
FetchContent_MakeAvailable(vc_ltl)
include("${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")

if (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.01 /SUBSYSTEM:WINDOWS,5.01 /ENTRY:\"mainCRTStartup\"" CACHE STRING "" FORCE)
else()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:CONSOLE,5.02 /SUBSYSTEM:WINDOWS,5.02 /ENTRY:\"mainCRTStartup\"" CACHE STRING "" FORCE)
endif()

message(STATUS "Windows XP MSVC Toolchain configured")
message(STATUS "  Platform Toolset: ${CMAKE_GENERATOR_TOOLSET}")
message(STATUS "  Platform:         ${CMAKE_GENERATOR_PLATFORM}")
message(STATUS "  Runtime Library:  ${CMAKE_MSVC_RUNTIME_LIBRARY}")
message(STATUS "  Build Type:       ${CMAKE_BUILD_TYPE}")
