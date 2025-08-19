# YY-Thunks wrapper

if(MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
    include(FetchContent)
    set(YY_Thunks_VERSION v1.1.7)
    FetchContent_Declare(YY_Thunks
        URL https://github.com/Chuyu-Team/YY-Thunks/releases/download/${YY_Thunks_VERSION}/YY-Thunks-Objs.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
    )
    FetchContent_MakeAvailable(YY_Thunks)

    add_library(YY_Thunks INTERFACE)

    if (CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(YY_Thunks INTERFACE
            ${YY_Thunks_SOURCE_DIR}/objs/x64/YY_Thunks_for_WinXP.obj
        )
    else()
        target_link_libraries(YY_Thunks INTERFACE
            ${YY_Thunks_SOURCE_DIR}/objs/x86/YY_Thunks_for_WinXP.obj
        )
    endif()

    message(STATUS "[AUI] Import: YY_Thunks")
endif()
