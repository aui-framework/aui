cmake_minimum_required(VERSION 3.10)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

aui_executable(aui.example.minesweeper)
aui_compile_assets(aui.example.minesweeper)

if (MSVC AND AUI_BUILD_FOR STREQUAL "winxp")
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
aui_link(aui.example.minesweeper PRIVATE aui::core aui::views YY_Thunks)
set_target_properties(aui.example.minesweeper PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
aui_link(aui.example.minesweeper PRIVATE aui::core aui::views)
endif()
