project(aui.example.game_of_life)
cmake_minimum_required(VERSION 3.16)

if (NOT (AUI_PLATFORM_WIN OR AUI_PLATFORM_LINUX OR AUI_PLATFORM_MACOS))
    return()
endif ()

# Uncomment this code to pull AUI:
#
# file(
#         DOWNLOAD
#         https://raw.githubusercontent.com/aui-framework/aui/master/aui.boot.cmake
#         ${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
# include(${CMAKE_CURRENT_BINARY_DIR}/aui.boot.cmake)
#
# auib_import(aui https://github.com/aui-framework/aui
#             COMPONENTS core views json)

aui_executable(aui.example.game_of_life)
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
aui_link(aui.example.game_of_life PRIVATE aui::views YY_Thunks)
set_target_properties(aui.example.game_of_life PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS,5.01 /ENTRY:WinMainCRTStartup")
else()
aui_link(aui.example.game_of_life PRIVATE aui::views)
endif()

