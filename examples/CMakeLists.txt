if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
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
    # if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    #     set(CMAKE_TOOLCHAIN_FILE ${AUI_BUILD_AUI_ROOT}/cmake/toolchains/winxp-x86.cmake)
    # endif()
endif()
add_subdirectory(app)
add_subdirectory(ui)
add_subdirectory(7guis)
