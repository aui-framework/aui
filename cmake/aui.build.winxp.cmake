# YY-Thunks wrapper & VC-LTL

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
        set(CMAKE_TOOLCHAIN_FILE ${AUI_BUILD_AUI_ROOT}/cmake/toolchains/winxp-x86.cmake)
    endif()

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

    if (NOT(TARGET benchmark_main OR TARGET benchmark))
    link_libraries(YY_Thunks)
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

    message(STATUS "[AUI] Import: VC-LTL ${vc_ltl_SOURCE_DIR}/VC-LTL helper for cmake.cmake")
endif()
